#include <stdint.h>
#include "utils.h"
#include "fail.h"
#include "string.h"

namespace ps2general {
    const uint8_t COMMAND_PORT = 0x64;
    const uint8_t DATA_PORT = 0x60;
    const uint8_t DISABLE_PORT_1 = 0xAD;
    const uint8_t ENABLE_PORT_1 = 0xAE;

    const uint8_t DISABLE_PORT_2 = 0xA7;
    const uint8_t ENABLE_PORT_2 = 0xA8;

    const uint8_t READ_CONFIG_BYTE = 0x20;
    const uint8_t WRITE_CONFIG_BYTE = 0x20;

    const uint8_t SELF_TEST = 0xAA;
    const uint8_t TEST_PORT_1 = 0xAB;
    const uint8_t TEST_PORT_2 = 0xA9;

    const uint8_t RESET_PORT_1 = 0xFF;

    const uint8_t BEGIN_DEVICE_COMMAND = 0xD4;

    const uint8_t ACK = 0xFA;
    const uint8_t TIMEOUT_PSEUDO = 0x00;

    const uint32_t PS2_PRESENT_FLAG = 0x01;

    bool verifyPS2Controller(FADTTable* table) {
        return (table->flags & PS2_PRESENT_FLAG) != 0x00;
    }
    void sendCommand(uint8_t command) {
        outb(COMMAND_PORT,command);
    }
    void sendData(uint8_t data) {
        outb(DATA_PORT,data);
    }
    uint8_t readStatusRegister() {
        return inb(COMMAND_PORT);
    }
    bool waitForReadReady(int timeout) {
        timeout *= 10;
        while ((readStatusRegister() & 0b00000001) == 0) {
            if (timeout != -10) {
                timeout--;
                if (timeout == 0) {
                    return false;
                }
            }
        }
        return true;
    }
    bool waitForWriteReady(int timeout) {
        timeout *= 10;
        while ((readStatusRegister() & 0b00000010) == 0) {
            if (timeout != -10) {
                timeout--;
                if (timeout == 0) {
                    return false;
                }
            }
        }
        return true;
    }
    void sendCommand16(uint8_t first, uint8_t second) {
        sendCommand(first);
        while ((readStatusRegister() & 0b00000010) != 0) {}
        sendData(second);
    }
    uint8_t sendCommandAck(uint8_t command) {
        sendCommand(command);
        if (waitForReadReady(1000) == false) {
            return TIMEOUT_PSEUDO;
        }
        uint8_t ack = inb(0x60);
        return ack;
    }
    uint8_t sendDataAck(uint8_t data) {
        sendData(data);
        if (waitForReadReady(1000) == false) {
            return TIMEOUT_PSEUDO;
        }
        uint8_t ack = inb(0x60);
        return ack;
    }
    uint8_t fetchConfigurationByte() {
        outb(COMMAND_PORT,READ_CONFIG_BYTE);
        if (waitForReadReady(1000) == false) return TIMEOUT_PSEUDO;
        return inb(DATA_PORT);
    }
    PS2Returns init(FADTTable* table) {
        cli();
        if (not verifyPS2Controller(table)) {
            return NoPS2Controller;
        }

        // DISABLE PORTS

        sendCommand(DISABLE_PORT_1);
        io_wait();
        sendCommand(DISABLE_PORT_2);

        // FLUSH PORT 0x60

        inb(DATA_PORT);
 // IRQ8 to 15
        // UPDATE CONFIGURATION

        uint8_t config = fetchConfigurationByte();

        if (config == TIMEOUT_PSEUDO) {
            return PS2Timeout;
        }

        config &= 0b00110100; // clear bits 0, 4, and 6 -- WRONG, CLEAR 0, 1, 4, and 6 (0b01101011)

        sendCommand16(WRITE_CONFIG_BYTE,config);

        // SELF TEST

        while (inb(0x64) & 2) {}

        sendCommand(SELF_TEST);
        if (not waitForReadReady(10000)) {
            return PS2Timeout;
        }
        uint8_t result = inb(DATA_PORT);
        if (result != 0x55) {
            return SelfTestFailure;
        }

        sendCommand16(WRITE_CONFIG_BYTE,config); // fix the configuration because sometimes selftest resets the config

        // CHECK FOR DUAL CHANNEL

        sendCommand(ENABLE_PORT_2);
        config = fetchConfigurationByte();
        bool dualChannel = false;
        if ((config & 0b00010000) == 0) {
            dualChannel = true;
            sendCommand(DISABLE_PORT_2);
        }

        // TEST PORTS

        bool portOneWorking = false;
        bool portTwoWorking = false;
        sendCommand(TEST_PORT_1);
        if (not waitForReadReady(10000)) {
            return PS2Timeout;
        }
        result = inb(DATA_PORT);
        if (result == 0x00) portOneWorking = true;



        sendCommand(TEST_PORT_2);
        if (not waitForReadReady(10000)) {
            return PS2Timeout;
        }
        result = inb(DATA_PORT);
        if (result == 0x00) portTwoWorking = true;

        if (not (portOneWorking or portTwoWorking)) {
            return NoWorkingPorts;
        }

        // Enable working ports, and their IRQs.

        config = fetchConfigurationByte();

        if (portOneWorking) {
            Terminal::printdebug("Enabled port one.\n");
            sendCommand(ENABLE_PORT_1);
            config |= 0b00010001;
        }
        if (portTwoWorking) {
            Terminal::printdebug("Enabled port two.\n");
            sendCommand(ENABLE_PORT_2);
            config |= 0b00100010;
        }

        sendCommand16(WRITE_CONFIG_BYTE,config); // update the IRQs in the config

        uint8_t responses[2] = {};
        bool portOnePopulated = true;
        bool portTwoPopulated = true;
        uint8_t portOneID = 0x00;
        uint8_t portTwoID = 0x00;
        if (portOneWorking) {
            sendCommand(RESET_PORT_1);
            if (not waitForReadReady(10000)) {
                portOnePopulated = false;
            } else {
                responses[1] = inb(DATA_PORT);
                if (responses[1] == 0xFC) {
                    portOnePopulated = false;
                    portOneWorking = false;
                } else {
                    if (not waitForReadReady(30000)) {
                        portOnePopulated = false;
                        portOneWorking = false;
                        // unexpected; it was working before
                    }
                    responses[2] = inb(DATA_PORT);
                }
            }
            if (portOneWorking and portOnePopulated) {
                if ((responses[0] == 0xFA and responses[1] == 0xAA) or 
                    (responses[0] == 0xAA and responses[1] == 0xFA)) {
                        if (not waitForReadReady(30000)) {
                            portOnePopulated = false;
                            portOneWorking = false;
                        } else {
                            portOneID = inb(DATA_PORT);
                        }
                    } 
            }
        }

        return Success;
    }
}

namespace ps2keyboard {

    char* str = "testingonetwothree";
    const uint8_t ENABLE_SCANNING = 0xF4;
    const uint8_t DISABLE_SCANNING = 0xF5;
    const uint8_t SELECT_SCANCODE = 0xF0;
    ps2stateMachine state; // state machine
    PS2_STATES oldState;
    PS2Returns codeStore;

    bool keysDown[0xFF] = {};

    Pair<uint64_t,KeyCode> set1codes[] = {
        {0x01,NAVIG_ESCAPE},
        {0x02,MATHS_1},
        {0x03,MATHS_2},
        {0x04,MATHS_3},
        {0x05,MATHS_4},
        {0x06,MATHS_5},
        {0x07,MATHS_6},
        {0x08,MATHS_7},
        {0x09,MATHS_8},
        {0x0A,MATHS_9},
        {0x0B,MATHS_0},
        {0x0C,MATHS_MINUS},
        {0x0D,MATHS_EQUALS},
        {0x0E,USAGE_BACKSPACE},
        {0x0F,SYMBL_TAB},
        {0x10,ALPHA_Q},
        {0x11,ALPHA_W},
        {0x12,ALPHA_E},
        {0x13,ALPHA_R},
        {0x14,ALPHA_T},
        {0x15,ALPHA_Y},
        {0x16,ALPHA_U},
        {0x17,ALPHA_I},
        {0x18,ALPHA_O},
        {0x19,ALPHA_P},
        {0x1A,SYMBL_OPEN_BRACKET},
        {0x1B,SYMBL_CLOSE_BRACKET},
        {0x1C,USAGE_ENTER},
        {0x1D,MODIF_LEFT_CONTROL},
        {0x26,ALPHA_L},
        {0x27,SYMBL_SEMICOLON},
        {0x28,SYMBL_APOSTROPHE},
        {0x1E,ALPHA_A},
        {0x1F,ALPHA_S},
        {0x20,ALPHA_D},
        {0x21,ALPHA_F},
        {0x22,ALPHA_G},
        {0x23,ALPHA_H},
        {0x24,ALPHA_J},
        {0x25,ALPHA_K},
        {0x26,ALPHA_L},
        {0x27,SYMBL_SEMICOLON},
        {0x28,SYMBL_APOSTROPHE},
        {0x29,SYMBL_BACKTICK},
        {0x2A,MODIF_LEFT_SHIFT},
        {0x2B,SYMBL_BACKSLASH},
        {0x2C,ALPHA_Z},
        {0x2D,ALPHA_X},
        {0x2E,ALPHA_C},
        {0x2F,ALPHA_V},
        {0x30,ALPHA_B},
        {0x31,ALPHA_N},
        {0x32,ALPHA_M},
        {0x33,SYMBL_COMMA},
        {0x34,SYMBL_PERIOD},
        {0x35,SYMBL_SLASH},
        {0x36,MODIF_RIGHT_SHIFT},
        {0x37,MATHS_KP_ASTERISK},
        {0x38,MODIF_LEFT_ALT},
        {0x39,ALPHA_SPACE},
        {0x3A,USAGE_LOCK_CAPS},
        {0x3B,USAGE_F1},
        {0x3C,USAGE_F2},
        {0x3D,USAGE_F3},
        {0x3E,USAGE_F4},
        {0x3F,USAGE_F5},
        {0x40,USAGE_F6},
        {0x41,USAGE_F7},
        {0x42,USAGE_F8},
        {0x43,USAGE_F9},
        {0x44,USAGE_F10},
        {0x45,USAGE_LOCK_NUMBER},
        {0x46,USAGE_LOCK_SCROLL},
        {0x47,MATHS_KP_7},
        {0x48,MATHS_KP_8},
        {0x49,MATHS_KP_9},
        {0x4A,MATHS_KP_MINUS},
        {0x4B,MATHS_KP_4},
        {0x4C,MATHS_KP_5},
        {0x4D,MATHS_KP_6},
        {0x4E,MATHS_KP_PLUS},
        {0x4F,MATHS_KP_1},
        {0x50,MATHS_KP_2},
        {0x51,MATHS_KP_3},
        {0x52,MATHS_KP_0},
        {0x53,MATHS_KP_PERIOD},
        {0x57,USAGE_F11},
        {0x58,USAGE_F12},

        // 0xE0 keys

        {0xE010,MEDIA_PREVIOUS_TRACK},
        {0xE019,MEDIA_NEXT_TRACK},
        {0xE01C,MATHS_KP_ENTER},
        {0xE01D,MODIF_RIGHT_CONTROL},
        {0xE020,MEDIA_MUTE},
        {0xE021,MEDIA_CALCULATOR},
        {0xE022,MEDIA_PLAY},
        {0xE024,MEDIA_STOP},
        {0xE02E,MEDIA_VOLUME_DOWN},
        {0xE030,MEDIA_VOLUME_UP},
        {0xE032,MEDIA_WWW_HOME},
        {0xE035,MATHS_KP_SLASH},
        {0xE038,MODIF_RIGHT_ALT},
        {0xE047,NAVIG_HOME},
        {0xE048,NAVIG_ARROW_UP},
        {0xE049,NAVIG_PAGE_UP},
        {0xE04B,NAVIG_ARROW_LEFT},
        {0xE04D,NAVIG_ARROW_RIGHT},
        {0xE04F,NAVIG_END},
        {0xE050,NAVIG_ARROW_DOWN},
        {0xE051,NAVIG_PAGE_DOWN},
        {0xE052,NAVIG_INSERT},
        {0xE053,NAVIG_DELETE},
        {0xE05B,MODIF_LEFT_META},
        {0xE05C,MODIF_RIGHT_META},
        {0xE05D,USAGE_MENU},
        {0xE05E,CNTRL_POWER},
        {0xE05F,CNTRL_SLEEP},
        {0xE063,CNTRL_WAKE},
        {0xE065,MEDIA_WWW_SEARCH},
        {0xE066,MEDIA_WWW_FAVORITES},
        {0xE067,MEDIA_WWW_REFRESH},
        {0xE068,MEDIA_WWW_STOP},
        {0xE069,MEDIA_WWW_FORWARD},
        {0xE06A,MEDIA_WWW_BACK},
        {0xE06B,MEDIA_MY_COMPUTER},
        {0xE06C,MEDIA_EMAIL},
        {0xE06D,MEDIA_SELECT},
        {0xE02AE037,USAGE_PRTSC},
        {0xE0B7E0AA,USAGE_PRTSC},
        {0xE11D45E19DC5,USAGE_PAUSE},
    };

    void flushBuffer() {
        while (inb(0x64) & 1) inb(0x60);
    }

    PS2Returns waitReady(int timeout) {
        while ((inb(0x64) & 0x02) != 0) {
            timeout--;
            if (timeout <= 0) {
                break;
            }
            asm("");
        }
        if (timeout <= 0) {
            return PS2Timeout;
        }
        return Success;
    }

    volatile uint8_t waitResponse(int timeout) {
        while (state.state == WaitingForAck) {
            timeout--;
            if (timeout <= 0) {
                break;
            }
            asm("");
        }
        if (state.state == WaitingForAck) {
            Terminal::printdebug("waitResponse timed out\n");
            return 0x00;
        }
        return state.data[0];

    }

    volatile PS2Returns waitAck(int timeout) {
        while (state.state == WaitingForAck) {
            timeout--;
            if (timeout <= 0) {
                break;
            }
            asm("");
        }
        if (state.state == WaitingForAck) {
            Terminal::printdebug("waitAck timed out\n");
            return PS2Timeout;
        }
        if (state.data[0] == ps2general::ACK) {
            Terminal::printdebug("waitAck success!\n");
            state.data[0] = 0;
            return Success;
        }
        Terminal::printdebug("waitAck got wrong value!\n0x");
            Terminal::printdebug(parseInt(state.data[0],str,16));
            Terminal::printdebug("\n");
        return NoAck;
    }

    PS2Returns disableScanning() {
        oldState = state.state;
        state.state = WaitingForAck;
        codeStore = waitReady(1000);
        if (codeStore != Success) return codeStore;
        ps2general::sendData(DISABLE_SCANNING);
        codeStore = waitAck(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("DSSCAN SUCCESS\n");
        state.state = oldState;
        return Success;
    }

    PS2Returns enableScanning() {
        oldState = state.state;
        state.state = WaitingForAck;
        codeStore = waitReady(1000);
        if (codeStore != Success) return codeStore;
        ps2general::sendData(ENABLE_SCANNING);
        codeStore = waitAck(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("ENSCAN SUCCESS\n");
        state.state = oldState;
        return Success;
    }

    volatile PS2Returns changeScancodeSet(uint8_t scancodeSet) {
        codeStore = disableScanning();
        if (codeStore != Success) return codeStore;
        oldState = state.state;
        state.state = WaitingForAck;
        Terminal::printdebug("Checking status...\n");
        Terminal::printdebug("Waited for ready successfully.\n");
        flushBuffer();
        ps2general::sendData(SELECT_SCANCODE);
        codeStore = waitAck(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("Waited for ack successfully.\n");

        flushBuffer();

        state.state = WaitingForAck;
        codeStore = waitReady(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("Waited for ready successfully.\n");
        flushBuffer();
        ps2general::sendData(scancodeSet);
        codeStore = waitAck(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("Waited for ack successfully.\n");

        state.state = oldState;

        state.scancodeSet = scancodeSet;

        Terminal::printdebug("WE'RE RETURNING SUCCESS, IT SHOULD NOT BE ERRORING!\n");
        codeStore = enableScanning();
        if (codeStore != Success) return codeStore;
        return codeStore;
    }

    uint8_t checkScancodeSet() {
        oldState = state.state;
        state.state = WaitingForAck;
        Terminal::printdebug("Checking status...\n");
        Terminal::printdebug("Waited for ready successfully.\n");
        flushBuffer();
        ps2general::sendData(SELECT_SCANCODE);
        codeStore = waitAck(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("Waited for ack successfully.\n");

        flushBuffer();

        state.state = WaitingForAck;
        codeStore = waitReady(1000);
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("Waited for ready successfully.\n");
        flushBuffer();
        ps2general::sendData(0x00);
        codeStore = waitAck(1000);
        state.state = WaitingForAck;
        if (codeStore != Success) return codeStore;
        Terminal::printdebug("Waited for ack successfully.\n");
        uint8_t scancodeSet = waitResponse(1000);

        state.state = oldState;
        return scancodeSet;
    }

    PS2Returns init(bool portTwo) {
        state.lastScancode = 0;
        state.firstScancode = 0;
        state.ready = true;
        sti();
        codeStore = changeScancodeSet(2);
        codeStore = changeScancodeSet(2);
        Terminal::printdebug("\n\n\nAAA\n\n\n");
        state.state = WaitingForScancodes;
        return codeStore;
    }

    // PS2Returns init(bool portTwo) {
    //     if (portTwo == false) {
    //         state.state = WaitingForAck;
    //         while (inb(0x64) & 1) inb(0x60); // flush the buffer
    //         ps2general::sendData(ENABLE_SCANNING);
    //         sti();
    //         uint8_t timeout = 100000;
    //         while (state.state == WaitingForAck) {
    //             timeout--;
    //             if (timeout <= 0) {
    //                 break;
    //             }
    //         }
    //         if (state.state == NoProcess) {
    //             // we recieved data!
    //             if (state.data[0] == ps2general::ACK) {
    //                 Terminal::printdebug("acked\n");
    //                 codeStore = changeScancodeSet(2);
    //                 // if (codeStore != Success) return codeStore;
    //                 state.state = WaitingForScancodes;
    //                 state.data[0] = 0;
    //                 state.lastScancode = 0;
                    
    //                 return Success;
    //             }
    //             return NoAck;
    //         } else if (state.state == WaitingForAck) {
    //             return PS2Timeout;
    //         }
    //         return UnexpectedState;
    //     }
    // }

    KeyAction scancodeToKeycode(uint8_t set,uint64_t scancode) {
        for (Pair<uint64_t,KeyCode> pair : set1codes) {
            if (scancode == pair.first) {
                return {true,pair.second,PRESSED};
            }
            if (scancode-0x80 == pair.first) {
                return {true,pair.second,RELEASED};
            }
        }
        return {false};
    }

    PS2Returns processScancodes() {
        if (state.state == Failure) {
            fault(-402,"PS/2 Keyboard state set to Failure.");
        }
        if (state.state == Unimplemented) {
            fault(-402,"PS/2 Keyboard state set to Unimplemented.");
        }
        uint8_t code = state.data[state.firstScancode];
        if (state.firstScancode != state.lastScancode) {
            state.currentData <<= 8;
            state.currentData |= code;
            // state.data[state.firstScancode] = 0;
            state.firstScancode = (state.firstScancode+1)%64;
        }
        KeyAction action = scancodeToKeycode(state.scancodeSet,state.currentData);
        if (!action.initialized) {
            return UnexpectedState; // The scancode is incomplete or invalid.
        }
        keysDown[action.code] = action.mode == PRESSED;
        keysDown[MODIF_ANY_ALT] = keysDown[MODIF_LEFT_ALT] or keysDown[MODIF_RIGHT_ALT];
        keysDown[MODIF_ANY_SHIFT] = keysDown[MODIF_LEFT_SHIFT] or keysDown[MODIF_RIGHT_SHIFT];
        keysDown[MODIF_ANY_CONTROL] = keysDown[MODIF_LEFT_CONTROL] or keysDown[MODIF_RIGHT_CONTROL];
        keysDown[MODIF_ANY_META] = keysDown[MODIF_LEFT_META] or keysDown[MODIF_RIGHT_META];
        state.state = WaitingForScancodes;
        state.currentData = 0;

        return Success;
    }

    // PS2Returns processScancodes() {
    //     if (state.state == EatingScancode) {
    //         // Terminal::printdebug("Scancode!\n");
    //         // Terminal::printdebug("Scanned: ");
    //         // Terminal::printdebug(parseInt(state.data[0],str,16));
    //         // if (state.data[0] > 0xFF) {
    //         //     Terminal::printdebug(" (this should be larger than 0xFF)");
    //         // } // E0 2A E0 37    E0 B7 E0 AA
    //         // if (state.data[0] == 0xE02AE037) {
    //         //     Terminal::printdebug(" (prtsc pressed)");
    //         // }
    //         // if (state.data[0] == 0xE0B7E0AA) {
    //         //     Terminal::printdebug(" (prtsc released)");
    //         // }
    //         // Terminal::printdebug("\n");
    //         Pair<KeyCode,KeyMode> keypair = scancodeToKeycode(state.scancodeSet,state.data[0]);
    //         keysDown[keypair.first] = keypair.second == PRESSED;
    //         keysDown[MODIF_ANY_ALT] = keysDown[MODIF_LEFT_ALT] or keysDown[MODIF_RIGHT_ALT];
    //         keysDown[MODIF_ANY_SHIFT] = keysDown[MODIF_LEFT_SHIFT] or keysDown[MODIF_RIGHT_SHIFT];
    //         keysDown[MODIF_ANY_CONTROL] = keysDown[MODIF_LEFT_CONTROL] or keysDown[MODIF_RIGHT_CONTROL];
    //         keysDown[MODIF_ANY_META] = keysDown[MODIF_LEFT_META] or keysDown[MODIF_RIGHT_META];
    //         state.data[0] = state.data[1];
    //         state.data[1] = state.data[2];
    //         state.data[2] = state.data[3];
    //         state.data[3] = 0;
    //         state.lastScancode--;
    //         if (state.data[0] == 0x00) {
    //             state.state = WaitingForScancodes;
    //         }
    //     }
    //     return Success;
    // }
}