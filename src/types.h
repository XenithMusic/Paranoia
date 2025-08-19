#include "stdint.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


#pragma once

struct spinlock_t {
    bool locked;
    void* resource;
};

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info;

enum MBlockState : char {
    USED = 0,
    FREE = 1,
    RSRV = 2
};

typedef struct MBlock {
    bool isEnd = true; // if false, there is another block in this sequence.
    enum MBlockState state = RSRV;
};

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};


enum PS2_STATES {
    NoProcess,
    WaitingForAck,
    EnablingScanning,
    WaitingForScancodes,
    EatingScancode,
    WaitingForPart,
};

struct ps2stateMachine {
    PS2_STATES state = NoProcess;
    uint8_t queueSize = 0;
    uint64_t data[4] = {};
    uint8_t stateInfo1 = 0;
    uint8_t stateInfo2 = 0;
    uint8_t scancodeSet = 0x01;
    bool overwhelmed = 0;
};

template <typename First,typename Second>
struct Pair {
    First first;
    Second second;
};

enum KeyCode {
    // Alphabetical
    ALPHA_A=0b00000000,
    ALPHA_B,
    ALPHA_C,
    ALPHA_D,
    ALPHA_E,
    ALPHA_F,
    ALPHA_G,
    ALPHA_H,
    ALPHA_I,
    ALPHA_J,
    ALPHA_K,
    ALPHA_L,
    ALPHA_M,
    ALPHA_N,
    ALPHA_O,
    ALPHA_P,
    ALPHA_Q,
    ALPHA_R,
    ALPHA_S,
    ALPHA_T,
    ALPHA_U,
    ALPHA_V,
    ALPHA_W,
    ALPHA_X,
    ALPHA_Y,
    ALPHA_Z,
    ALPHA_SPACE,
    // Mathematical (keypad and their non-keypad equivalents)
    MATHS_0=0b00100000,
    MATHS_1,
    MATHS_2,
    MATHS_3,
    MATHS_4,
    MATHS_5,
    MATHS_6,
    MATHS_7,
    MATHS_8,
    MATHS_9,
    MATHS_MINUS,
    MATHS_EQUALS,
    MATHS_KP_0,
    MATHS_KP_1,
    MATHS_KP_2,
    MATHS_KP_3,
    MATHS_KP_4,
    MATHS_KP_5,
    MATHS_KP_6,
    MATHS_KP_7,
    MATHS_KP_8,
    MATHS_KP_9,
    MATHS_KP_SLASH,
    MATHS_KP_ASTERISK,
    MATHS_KP_MINUS,
    MATHS_KP_PLUS,
    MATHS_KP_ENTER,
    MATHS_KP_PERIOD,
    // SYMBOLIC
    SYMBL_BACKTICK=0b01000000, // `
    SYMBL_OPEN_BRACKET, // [
    SYMBL_CLOSE_BRACKET, // ]
    SYMBL_BACKSLASH,
    SYMBL_SEMICOLON,
    SYMBL_APOSTROPHE,
    SYMBL_COMMA,
    SYMBL_SLASH,
    SYMBL_PERIOD,
    SYMBL_TAB,
    // MODIFIER
    MODIF_LEFT_SHIFT=0b01100000,
    MODIF_LEFT_CONTROL,
    MODIF_LEFT_META,
    MODIF_LEFT_ALT,
    MODIF_RIGHT_ALT,
    MODIF_RIGHT_META,
    MODIF_RIGHT_CONTROL,
    MODIF_RIGHT_SHIFT,
    MODIF_ANY_SHIFT,
    MODIF_ANY_CONTROL,
    MODIF_ANY_META,
    MODIF_ANY_ALT,
    // USAGE
    USAGE_BACKSPACE=0b10000000,
    USAGE_ENTER,
    USAGE_MENU,
    USAGE_LOCK_CAPS,
    USAGE_LOCK_SCROLL,
    USAGE_LOCK_NUMBER,
    USAGE_PRTSC,
    USAGE_PAUSE,
    USAGE_F1,
    USAGE_F2,
    USAGE_F3,
    USAGE_F4,
    USAGE_F5,
    USAGE_F6,
    USAGE_F7,
    USAGE_F8,
    USAGE_F9,
    USAGE_F10,
    USAGE_F11,
    USAGE_F12,
    USAGE_F13,
    USAGE_F14,
    USAGE_F15,
    USAGE_F16,
    USAGE_F17,
    USAGE_F18,
    USAGE_F19,
    USAGE_F20,
    USAGE_F21,
    USAGE_F22,
    USAGE_F23,
    USAGE_F24,
    // NAVIGATION
    NAVIG_HOME=0b10100000,
    NAVIG_END,
    NAVIG_PAGE_UP,
    NAVIG_PAGE_DOWN,
    NAVIG_ARROW_UP,
    NAVIG_ARROW_LEFT,
    NAVIG_ARROW_DOWN,
    NAVIG_ARROW_RIGHT,
    NAVIG_ESCAPE,
    NAVIG_INSERT,
    NAVIG_DELETE,
    // MEDIA
    MEDIA_PREVIOUS_TRACK=0b11000000,
    MEDIA_NEXT_TRACK,
    MEDIA_MUTE,
    MEDIA_CALCULATOR,
    MEDIA_PLAY,
    MEDIA_STOP,
    MEDIA_VOLUME_DOWN,
    MEDIA_VOLUME_UP,
    MEDIA_WWW_HOME,
    MEDIA_WWW_SEARCH,
    MEDIA_WWW_FAVORITES,
    MEDIA_WWW_REFRESH,
    MEDIA_WWW_STOP,
    MEDIA_WWW_FORWARD,
    MEDIA_WWW_BACK,
    MEDIA_MY_COMPUTER,
    MEDIA_EMAIL,
    MEDIA_SELECT,
    // CONTROL
    CNTRL_POWER=0b11100000,
    CNTRL_SLEEP,
    CNTRL_WAKE,
};

enum KeyMode {
    PRESSED,
    RELEASED
};

const Pair<KeyCode,char> codesAscii[] = {
    {ALPHA_A,'a'},
    {ALPHA_B,'b'},
    {ALPHA_C,'c'},
    {ALPHA_D,'d'},
    {ALPHA_E,'e'},
    {ALPHA_F,'f'},
    {ALPHA_G,'g'},
    {ALPHA_H,'h'},
    {ALPHA_I,'i'},
    {ALPHA_J,'j'},
    {ALPHA_K,'k'},
    {ALPHA_L,'l'},
    {ALPHA_M,'m'},
    {ALPHA_N,'n'},
    {ALPHA_O,'o'},
    {ALPHA_P,'p'},
    {ALPHA_Q,'q'},
    {ALPHA_R,'r'},
    {ALPHA_S,'s'},
    {ALPHA_T,'t'},
    {ALPHA_U,'u'},
    {ALPHA_V,'v'},
    {ALPHA_W,'w'},
    {ALPHA_X,'x'},
    {ALPHA_Y,'y'},
    {ALPHA_Z,'z'},
    {ALPHA_SPACE,' '},
    {USAGE_ENTER,'\n'},
    {MATHS_1,'1'},
    {MATHS_2,'2'},
    {MATHS_3,'3'},
    {MATHS_4,'4'},
    {MATHS_5,'5'},
    {MATHS_6,'6'},
    {MATHS_7,'7'},
    {MATHS_8,'8'},
    {MATHS_9,'9'},
    {MATHS_0,'0'},
    {MATHS_KP_0,'0'},
    {MATHS_KP_1,'1'},
    {MATHS_KP_2,'2'},
    {MATHS_KP_3,'3'},
    {MATHS_KP_4,'4'},
    {MATHS_KP_5,'5'},
    {MATHS_KP_6,'6'},
    {MATHS_KP_7,'7'},
    {MATHS_KP_8,'8'},
    {MATHS_KP_9,'9'},
    {SYMBL_BACKTICK,'`'},
    {MATHS_MINUS,'-'},
    {MATHS_EQUALS,'='},
};

const Pair<KeyCode,char> codesAsciiCapital[] = {
        {ALPHA_A,'A'},
        {ALPHA_B,'B'},
        {ALPHA_C,'C'},
        {ALPHA_D,'D'},
        {ALPHA_E,'E'},
        {ALPHA_F,'F'},
        {ALPHA_G,'G'},
        {ALPHA_H,'H'},
        {ALPHA_I,'I'},
        {ALPHA_J,'J'},
        {ALPHA_K,'K'},
        {ALPHA_L,'L'},
        {ALPHA_M,'M'},
        {ALPHA_N,'N'},
        {ALPHA_O,'O'},
        {ALPHA_P,'P'},
        {ALPHA_Q,'Q'},
        {ALPHA_R,'R'},
        {ALPHA_S,'S'},
        {ALPHA_T,'T'},
        {ALPHA_U,'U'},
        {ALPHA_V,'V'},
        {ALPHA_W,'W'},
        {ALPHA_X,'X'},
        {ALPHA_Y,'Y'},
        {ALPHA_Z,'Z'},
        {ALPHA_SPACE,' '},
        {USAGE_ENTER,'\n'},
        {MATHS_1,'!'},
        {MATHS_2,'@'},
        {MATHS_3,'#'},
        {MATHS_4,'$'},
        {MATHS_5,'%'},
        {MATHS_6,'^'},
        {MATHS_7,'&'},
        {MATHS_8,'*'},
        {MATHS_9,'('},
        {MATHS_0,')'},
        {MATHS_KP_0,'0'},
        {MATHS_KP_1,'1'},
        {MATHS_KP_2,'2'},
        {MATHS_KP_3,'3'},
        {MATHS_KP_4,'4'},
        {MATHS_KP_5,'5'},
        {MATHS_KP_6,'6'},
        {MATHS_KP_7,'7'},
        {MATHS_KP_8,'8'},
        {MATHS_KP_9,'9'},
        {SYMBL_BACKTICK,'~'},
        {MATHS_MINUS,'_'},
        {MATHS_EQUALS,'+'},
    };

enum syscall_fail {
    SCFAIL_FAULT_FAILURE = 1000,

    SCFAIL_NOPERMISSION = -1000,
    SCFAIL_OUTOFTIME = -1001,
    SCFAIL_BOOSTUNAVAILABLE = -1002, // trust boost unavailable

    SCFAIL_USERCANCELLED = -2000,
    SCFAIL_RESERVED_0000 = -2001,
    SCFAIL_FILEMISSING = -2002,
    SCFAIL_USERFAILED = -2003, // wrong password?
    SCFAIL_AUTODECLINE = -2004,
    SCFAIL_BADINT = -2005,
    SCFAIL_BADPID = -2006,
    SCFAIL_BADFID = -2007,
    SCFAIL_BADENUM = -2008,
    SCFAIL_LOCKED = -2009,
    SCFAIL_KERNELMODE_DISABLED = -2010,
    SCFAIL_FILEEXISTS = -2011,
    SCFAIL_USERSPACE = -2012, // called in the kernel or in a driver for some reason
    SCFAIL_DISABLED = -2013, // reqkernel only. indicates that kernel mode was disabled.
    SCFAIL_DEADPID = -2014,
    SCFAIL_STOLENFID = -2015,
    SCFAIL_DEADFID = -2016,
    SCFAIL_TRUSTDECLINED = -2017, // haha you're broke you forgot to `boosttrust();`
    SCFAIL_NET_CONNECTIONREFUSED = -2018,
    SCFAIL_NET_CONNECTIONSTALE = -2019,
    SCFAIL_NET_CONNECTIONDEAD = -2020,
    SCFAIL_NET_CONNECTIONPLACEBO = -2021,
    SCFAIL_NET_CONNECTIONBANNED = -2022,
    SCFAIL_DNS_ROUTINGFAILED = -2023,
    SCFAIL_DNS_ROUTEMISSING = -2024,
    SCFAIL_BUFFEROVERFLOW = -2025,
    SCFAIL_OUT_OF_MEMORY = -2026,
    SCFAIL_RESERVED_0001 = -2027,
    SCFAIL_ILLEGALVALUE = -2028, // out of bounds.
    SCFAIL_POINTER_ILLEGALMEMORY = -2029,
    SCFAIL_POINTER_UNSAFEMEMORY = -2030,
    SCFAIL_POINTER_UNALLOCATED = -2031,
    INTERNAL_NUMBLOCKS_BADINT = -2032,
    INTERNAL_KERNELSIZE_OUT_OF_MEMORY = -2033,
};

enum syscall_id {
    SCALL_REQKERNEL = 100,
    SCALL_REQELEVATE = 101,
    SCALL_BOOSTTRUST = 102,
    SCALL_EXIT = 200,
    SCALL_PLACE = 201,
    SCALL_WAITPLACED = 202,
    SCALL_KILL = 203, // RE
    SCALL_ME = 204,
    SCALL_WHO = 205,
    SCALL_CHANGERUDE = 206,
    SCALL_SIGNAL = 207,
    SCALL_FETCHFILE = 300,
    SCALL_SETINTENT = 301,
    SCALL_READ = 302,
    SCALL_REPLACE = 303, // EC
    SCALL_APPEND = 304, // EC
    SCALL_DELETEFILE = 305, // RE
    SCALL_CREATEFILE = 306,
    SCALL_COPY = 307,
    SCALL_MOUNTFS = 308,
    SCALL_EJECTFS = 309, // RE
    SCALL_FSINFO = 310,
    SCALL_SLEEP = 400,
    SCALL_GETFAIL = 401,
    SCALL_YIELD = 402,
    SCALL_NETDEAD = 500,
    SCALL_NETCON = 501,
    SCALL_NETKILL = 502,
    SCALL_NETBIND = 503, // EC
    SCALL_NETACCEPT = 504,
    SCALL_NETSEND = 505,
    SCALL_NETRECV = 506,
    SCALL_NETINFO = 507,
    SCALL_DNSRES = 508,
    SCALL_DNSLOCAL = 509,
    SCALL_NETFIREWALL = 600, // RE
    SCALL_NETMONITOR = 601, // RE
    SCALL_GETNETMONITOR = 602, // RE
    SCALL_SETFILEPERM = 603, // RE
    SCALL_GETFILEPERM = 604,
    SCALL_MALLOC = 700,
    SCALL_MFREE = 701,
    SCALL_MINFO = 702,
    SCALL_LISTDEVICES = 800,
    SCALL_UPTIME = 900,
    SCALL_SHUTDOWN = 901, // EC
    SCALL_RESTART = 902, // EC
    SCALL_SUSPEND = 903 // EC
};

enum ps2device {
    ANCIENT = 0, // Ancient AT Keyboard
    PS2MOUSE = 1, // Standard PS/2 Mouse
    SCROLLMOUSE = 2, // Mouse with scroll wheel
    FIVEBTNMOUSE = 3, // 5 Button Mouse
    MF2KEYBOARD = 4, // MF2 Keyboard
    SHORTKEYBOARD = 5, // IBM Thinkpads, Spacesaver keyboards, Many other "short" keyboards
    NCDN97KEYBOARD = 6, // NCD N-97 KEYBOARD
    HOST122KEYBOARD = 7, // 122-Key Host Connected Keyboard
    KEY122KEYBOARD = 8, // 122-Key Keyboard
    JPKEYBOARD_G = 9, // Japanese "G" Keyboard
    JPKEYBOARD_P = 10, // Japanese "P" Keyboard
    JPKEYBOARD_A = 11, // Japanese "A" Keeyboard
    NCDSUNKEYBOARD = 12, // NCD Sun Layout Keyboard
};

// struct GDT {
//     uint32_t base;
//     uint32_t limit;
//     char access;
//     char flags;
// };

struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct GDTPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct IDTEntry {
    uint16_t offset_low;  // Lower 16 bits of the handler function address
    uint16_t selector;    // Code segment selector in GDT
    uint8_t zero;         // Reserved, set to 0
    uint8_t type_attr;    // Type and attributes (e.g., present, DPL, gate type)
    uint16_t offset_high; // Upper 16 bits of the handler function address
} __attribute__((packed));

struct IDTPointer {
    uint16_t limit; // Size of the IDT - 1
    uint32_t base;  // Address of the first IDTEntry
} __attribute__((packed));

struct stackframe {
    struct stackframe* ebp;
    uint32_t eip;
};

struct RSDPDescriptor {
	char signature[8];		// "RSD PTR "
	uint8_t checksum;		// Entire struct must sum to 0
	char oem_id[6];
	uint8_t revision;		// 0 = ACPI 1.0
	uint32_t rsdt_address;	// 32-bit physical address of RSDT
} __attribute__((packed));

// 0000

// 0000, 0000, 00, 00, 00, 00
// 0000, ffff, 00, cf, 9a, 00
// 0000, ffff, 00, cf, 92, 00

// ??

// 0020, 2230, 00, 78, 6a, b5