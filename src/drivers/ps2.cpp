#include <stdint.h>
#include "utils.h"
#include "fail.h"
#include "string.h"
enum PS2Returns {
    NoPS2Controller,
    PS2Timeout,
    SelfTestFailure,
    NoWorkingPorts,
    Success,
    NoAck,
    UnexpectedState,
};

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

    bool verifyPS2Controller() {
        return true; // TODO: HACK: VERY BAD PRACTICE. CHANGE THIS AS SOON AS POSSIBLE.
                     //             DO NOT ASSUME PS/2 CONTROLLER. MY HOSTCOMPUTER DOES NOT HAVE PS/2, AND WILL CRASH.
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
    PS2Returns init() {
        cli();
        if (not verifyPS2Controller()) {
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

        config &= 0b10000100; // clear bits 0, 4, and 6 -- WRONG, CLEAR 0, 1, 4, and 6 (0b01101011)

        sendCommand16(WRITE_CONFIG_BYTE,config);

        // SELF TEST

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
        if (config & 0b00010000 == 0) {
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
            Terminal::print("Enabled port one.\n");
            sendCommand(ENABLE_PORT_1);
            config |= 0b00010001;
        }
        if (portTwoWorking) {
            Terminal::print("Enabled port two.\n");
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

    const uint8_t ENABLE_SCANNING = 0xF4;
    ps2stateMachine state; // state machine

    PS2Returns init(bool portTwo) {
        if (portTwo == false) {
            state.state = EnablingScanning;
            while (inb(0x64) & 1) inb(0x60); // flush the buffer
            ps2general::sendData(ENABLE_SCANNING);
            sti();
            uint8_t timeout = 100000;
            while (state.state == EnablingScanning) {
                timeout--;
                if (timeout <= 0) {
                    break;
                }
            }
            if (state.state == NoProcess) {
                // we recieved data!
                if (state.data1 == ps2general::ACK) {
                    Terminal::print("acked\n");
                    state.state = WaitingForScancodes;
                    
                    return Success;
                }
                return NoAck;
            } else if (state.state == EnablingScanning) {
                return PS2Timeout;
            }
            return UnexpectedState;
        }
    }

    PS2Returns processScancodes() {
        if (state.state == EatingScancode) {
            Terminal::print("Scancode!");
            state.data1 = state.data2;
            state.data2 = 0x00;
            if (state.data1 == 0x00) {
                state.state = WaitingForScancodes;
            }
        }
        return Success;
    }
}