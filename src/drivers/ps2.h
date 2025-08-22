#include <stdint.h>
#include "utils.h"
enum PS2Returns {
    NoPS2Controller,
    PS2Timeout,
    SelfTestFailure,
    NoWorkingPorts,
    Success,
    NoAck,
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

    const uint8_t ACK = 0xFA;

    bool verifyPS2Controller();
    void sendCommand(uint8_t command);
    void sendData(uint8_t data);
    uint8_t readStatusRegister();
    bool waitForReadReady(int timeout);
    void sendCommand16(uint8_t first, uint8_t second);
    uint8_t sendCommandAck(uint8_t command);
    uint8_t fetchConfigurationByte();
    PS2Returns init(FADTTable* table);
}

namespace ps2keyboard {

    const uint8_t ENABLE_SCANNING = 0xFF;
    extern ps2stateMachine state; // state machine
    extern bool keysDown[0xFF];

    PS2Returns init(bool portTwo);
    PS2Returns processScancodes();
}