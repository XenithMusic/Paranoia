#include "utils.h"
#include "fail.h"
#include "terminal.h"
#include "string.h"

// DATA PORT: 0x60
// COMMAND PORT: 0x64

void out(bool isData, uint8_t data) {
    uint16_t port = 0x64;
    if (isData) port = 0x60;
    outb(port,data);
}

bool outack(bool isData, uint8_t data) {
    out(isData,data);
    uint8_t ack = inblocking(true);
    return ack == 0xFA;
}

uint8_t in(bool isData) {
    uint16_t port = 0x64;
    if (isData) port = 0x60;
    return inb(port);
}

uint8_t inblocking(bool isData) {
    while ((inb(0x64) & 0x01) == 0);
    return in(isData);
}

void flushOutBuf() {
    while (inb(0x64) & 1)
        inb(0x60);
    io_wait();
}

void disableDevices() {
    out(false,0xAD);
    io_wait();
    out(false,0xA7);
    io_wait();
}

uint8_t readConfig() {
    out(false,0x20);
    io_wait();
    return in(true);
}

void loadConfig(uint8_t data) {
    out(false,0x60);
    while (in(false) & 0x02 == 0);
    out(true,data);
}

uint8_t maskConfig(uint8_t keepOn, uint8_t turnOn) {
    // 0 bits in keepOn will be set to 0, 1 bits in turnOn will be set to 1.
    uint8_t config = readConfig();
    config = config & keepOn | turnOn;
    loadConfig(config);
    return config;
}

bool doesPS2Exist() {
    return true; // TODO: actual proper checks
}

bool selfTest() {
    out(false,0xAA);
    io_wait();
    return in(true) == 0x55;
}

bool hasChannel2() {
    out(false,0xA8); // enable second PS/2 port
    io_wait();
    bool response = readConfig() && 0b00001 == 0;
    if (response) {
        out(false,0xA7);
        maskConfig(0b101110111,0);
    }
    return response;
}

uint8_t testInterfaces(bool channel2Exists) {
    uint8_t ret = 0;
    out(false,0xAB);
    io_wait();
    bool portOne = in(true);
    bool portTwo = false;
    if (portOne == 0x00) {
        ret |= 0b00000001;
        Terminal::print("[  OK  ] PS/2 Port 1 Test\n");
    } else {
        Terminal::print("[ FAIL ] PS/2 Port 1 Test\n");
    }
    if (channel2Exists) {
        out(false,0xA9);
        io_wait();
        portTwo = in(true);
        if (portTwo == 0x00) {
            ret |= 0b00000010;
            Terminal::print("[  OK  ] PS/2 Port 2 Test\n");
        } else {
            Terminal::print("[ FAIL ] PS/2 Port 2 Test\n");
        }
    }
    return ret;
}

char* ps2kbstr; // pointer if anything demands a pointer
bool resetDevices() {
    bool hasAA = false;
    bool hasFA = false;
    uint8_t data;
    out(true,0xFF);
    data = inblocking(true);
    Terminal::print("DATA1: 0x");
    parseDouble((double)data,ps2kbstr,16);
    Terminal::print(ps2kbstr);
    Terminal::print("\n");
    if (data == 0xAA) { hasAA = true; } else if (data == 0xFA) { hasFA = true; };
    if (data == 0xFC) {
        Terminal::print("[ FAIL ] Devices failed to reset; self-test failure. \n");
        return false;
    }
    data = inblocking(true);
    Terminal::print("DATA2: 0x");
    parseDouble((double)data,ps2kbstr,16);
    Terminal::print(ps2kbstr);
    Terminal::print("\n");
    if (data == 0xAA) { hasAA = true; } else if (data == 0xFA) { hasFA = true; };
    if (hasAA) {
        Terminal::print("[  OK  ] Has AA\n");
    } else {
        Terminal::print("[ FAIL ] Doesn't have AA\n");
    }
    if (hasFA) {
        Terminal::print("[  OK  ] Has FA\n");
    } else {
        Terminal::print("[ FAIL ] Doesn't have FA\n");
    }
    return hasAA and hasFA;
}

namespace ps2ctl
{
    bool init() {
        // disable devices
        disableDevices();
        flushOutBuf();
        uint8_t config = maskConfig(0b00100110,0); // 0b01100100
        bool test = selfTest();
        if (!test) {
            Terminal::print("[ FAIL ] PS/2 Controller Self-Test\n");
        } else {
            Terminal::print("[  OK  ] PS/2 Controller Self-Test\n");
        }
        loadConfig(config); // do this again because sometimes the self test resets the PS/2 controller
        bool ch2 = hasChannel2();
        if (ch2) {
            Terminal::print("[ INFO ] PS/2 Controller has second port.\n");
        }
        uint8_t workingPorts = testInterfaces(ch2);
        if (workingPorts & 0b00000001 == 1) {
            out(false,0xAE); // enable port 1
            config = maskConfig(0b11111111,0b00000001);
        }
        if (ch2 and workingPorts & 0b00000010 == 1) {
            out(false,0xA8); // enable port 2
            config = maskConfig(0b11111111,0b00000010);
        }
        bool works = resetDevices();
        if (works) {
            Terminal::print("[  OK  ] PS/2 Driver Init\n");
            Terminal::print("[PS2CTL] Identifying PS/2 device");

            return true;
        } else {
            Terminal::print("[ FAIL ] PS/2 Driver\n");
            return false;
        }
    }
    /*
    bool init_keyboard() {
        if (outack(true,0xF4)) {
            Terminal::print("[ INFO ] Keyboard ACK'd 0xF4"); // send 0xF4 on data 
        } else {
            Terminal::print("[ FAIL ] Keyboard did not ACK 0xF4; keyboard input may not work.");
        }
    }
    */
}