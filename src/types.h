#include "stdint.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


#pragma once

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
    INTERNAL_KERNELSIZE_OUT_OF_MEMORY = -2033
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