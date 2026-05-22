#include "stdint.h"
#include "stddef.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

// #ifdef BITS64
// typedef unsigned long long size_t;
// #else
// #ifdef BITS32
// typedef unsigned int size_t;
// #else
// #error "I don't know if I should define size_t 32-bit or 64-bit!"
// #endif
// #endif

#pragma once

typedef uint64_t ubyte8_t;
typedef uint32_t ubyte4_t;
typedef uint16_t ubyte2_t;
typedef uint8_t ubyte1_t;

typedef ubyte1_t ubyte_t;

typedef uint32_t uid_t;
typedef uid_t fsid_t; // filesystem id
typedef uid_t fid_t; // file id
typedef uid_t pid_t; // process id

struct spinlock_t {
    bool locked;
    void* resource;
};

struct DriverInfo {
    int8_t priority;
    char ident[8];

    void* preinitFunction = nullptr;
    void* initFunction; // required
    void* postinitFunction = nullptr;
    
    void* preiterFunction = nullptr;
    void* iterFunction = nullptr;
    void* postiterFunction = nullptr;
};

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info;

struct Framebuffer {
    uint32_t framebuffer_addr;     // physical address of framebuffer
    uint32_t framebuffer_pitch;    // bytes per scanline
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;      // bits per pixel
    uint8_t  framebuffer_type;     // 0=Indexed, 1=RGB, 2=EGA Text
} __attribute__((packed));

// directly from chatgpt for testing to see if this is actually correct
typedef struct {
    uint32_t flags;        // bitfield
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // --- framebuffer info ---
    Framebuffer framebuffer;
    uint8_t  reserved[2];          // padding
} multiboot_info_t;

enum MBlockState : char {
    USED = 0,
    FREE = 1,
    RSRV = 2
};

struct MBlock {
    bool isEnd = true; // if false, there is another block in this sequence.
    enum MBlockState state = RSRV;
};

struct AllocHeader {
    size_t STARTBLOCK;
    size_t ENDBLOCK;
};

struct TSS32 {
    uint32_t prev_task_link;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t padding0;
    uint32_t esp1, esp2;
    uint16_t ss1, ss2;
    uint16_t padding1[5];
    uint32_t cr3, eip, eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t es, cs, ss, ds, fs, gs;
    uint16_t padding2[3];
    uint16_t io_map_base;
};

struct Date {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t mode;
};

namespace scheduler {
    struct Process {
        bool valid;
        // state
        uint32_t EAX;
        uint32_t EBX;
        uint32_t ECX;
        uint32_t EDX;
        uint32_t ESI;
        uint32_t EDI;
        uint32_t EBP;
        uint32_t ESP;
        // uint64_t R8;
        // uint64_t R9;
        // uint64_t R10;
        // uint64_t R11;
        // uint64_t R12;
        // uint64_t R13;
        // uint64_t R14;
        // uint64_t R15;

        uint32_t EIP;
        uint16_t CS;
        uint16_t DS;
        uint16_t SS;
        uint16_t ES;
        uint16_t FS;
        uint16_t GS;
        uint32_t EFLAGS;
        uint32_t CR3;
    };
};

namespace MBR {
    struct PTableEntry {
        ubyte1_t attributes;
        ubyte1_t ignored1[3];
        ubyte1_t type;
        ubyte1_t ignored2[3];
        ubyte4_t sectorStart;
        ubyte4_t sectorCount;
    } __attribute__((packed));
    struct MBRFooter {
        ubyte4_t signature;
        ubyte2_t reserved;
        PTableEntry partitionTable[4];
        ubyte2_t magic;
    } __attribute__((packed));
}

namespace ext2 {
    enum FilesystemState : ubyte2_t {
        CLEAN = 0x01,
        ERRORS = 0x02
    };
    enum ErrorHandler : ubyte2_t {
        // Ignore the errors.
        SKIP = 0x01,
        // Remount the file system as read-only.
        READONLY = 0x02,
        // Immediately trigger a Kernel Panic.
        FAULT = 0x03
    };
    enum OSIdentifier : ubyte4_t {
        LINUX = 0,
        HURD = 1,
        MASIX = 2,
        FREEBSD = 3,
        LITE = 4,
        PARANOID = 5,
    };
    // 17 ubyte4
    // 6 ubyte2
    // 
    struct Superblock {
        ubyte4_t total_inodes;
        ubyte4_t total_blocks;
        // Blocks reserved for superuser.
        ubyte4_t super_blocks;
        // Unallocated blocks
        ubyte4_t unall_blocks;
        // Unallocated inodes
        ubyte4_t unall_inodes;
        // Quoted directly from the osdev wiki:
        // Block number of the block containing the superblock (also the starting block number, NOT always zero.)
        // 
        // not sure what this means. will figure out later.
        ubyte4_t superblock_no;
        // This is not actually the block size; the block size can be optained with 1024 << this value
        ubyte4_t block_size;
        // This is not actually the fragment size; the fragment size can be optained with 1024 << this value
        ubyte4_t frag_size;
        // Number of blocks in a group
        ubyte4_t group_blocks;
        // Number of fragments in a group
        ubyte4_t group_frags;
        // Number of inodes in a group
        ubyte4_t group_inodes;
        // Last mount time. (since unix epoch)
        ubyte4_t last_mount;
        // Last write time. (since unix epoch)
        ubyte4_t last_write;
        // Number of mounts since the last consistency check.
        ubyte2_t mounts_since_fsck;
        // Number of mounts until a consistency check must be done.
        ubyte2_t mounts_to_fsck;

        // Should be 0xEF53. If it is not, this is not a valid ext2 filesystem.
        ubyte2_t signature;

        FilesystemState fs_state;

        ErrorHandler err_handler;

        ubyte2_t version_minor;

        // Last consistency check (since unix epoch)
        ubyte4_t last_fsck;
        // Interval between consistency checks (in seconds)
        ubyte4_t fsck_interval;

        // ID of the OS which created this volume.
        OSIdentifier os_creator;

        ubyte4_t version_major;

        // User ID that can use reserved blocks.
        ubyte2_t super_user;
        // Group ID that can use reserved blocks.
        ubyte2_t super_group;

        ubyte4_t first_free_inode;
        ubyte2_t inode_size;
        ubyte2_t block_group;
        ubyte4_t optional_features;
        ubyte4_t required_features;
        ubyte4_t writeable_features;
        ubyte8_t filesystem_id[2];
        char volume_name[16];
        char path_vol_last_mount[64];
        ubyte4_t compress_algorithm;
        ubyte_t file_early_blocks;
        ubyte_t dir_early_blocks;
        ubyte2_t reserved;
        ubyte8_t journal_id[2];
        ubyte4_t journal_inode;
        ubyte4_t journal_device;
        ubyte4_t head_orphans;
    } __attribute__((packed));

    struct BlockGroupDescriptor {
        // Address in blocks.
        ubyte4_t block_bitmap_addr;
        // Address in blocks.
        ubyte4_t inode_bitmap_addr;
        // Address in blocks.
        ubyte4_t inode_table_addr;
        ubyte2_t no_unallocated_blocks;
        ubyte2_t no_unallocated_inodes;
        ubyte2_t no_directories;
        ubyte1_t reserved[14];
    } __attribute__((packed));
    struct Inode {
        /**
         * Types and permissions.
         * 
         * 0x0FFF represent the permissions.
         * 0xF000 represent the types.
         * 
         * Permissions:
         * 0x1 = Other (execute)
         * 0x2 = Other (write)
         * 0x4 = Other (read)
         * 0x8 = Group (execute)
         * 0x10 = Group (write)
         * 0x20 = Group (read)
         * 0x40 = User (execute)
         * 0x80 = User (write)
         * 0x100 = User (read)
         * 0x200 = Sticky Bit
         * 0x400 = Set Group ID
         * 0x800 = Set User ID
         * 
         * Types:
         * 0x1000 = FIFO
         * 0x2000 = Character device
         * 0x4000 = Directory
         * 0x6000 = Block device
         * 0x8000 = Regular file
         * 0xA000 = Symbolic link
         * 0xC000 = Unix socket
         */
        ubyte2_t types_permissions;
        // poor name, sorry, dunno what this means. maybe the owner?
        ubyte2_t user_id;
        ubyte4_t lower_size;
        // (since unix epoch)
        ubyte4_t last_access;
        // Creation Time (since unix epoch)
        ubyte4_t first_write;
        // (since unix epoch)
        ubyte4_t last_modify;
        // (since unix epoch)
        ubyte4_t deletion_time;
        // poor name, sorry, dunno what this means. same as with user_id. maybe the owner?
        ubyte2_t group_id;
        // how many paths directly refer here
        ubyte2_t hard_links;
        // how many disk sectors are in use by this inode
        ubyte4_t disk_sectors;
        /**
         * 0x1 = Secure deletion (unused)
         * 0x2 = Keep a copy of data when deleted (unused)
         * 0x4 = File compression (unused)
         * 0x8 = Synchronous updates -- new data is immediately written
         * 0x10 = Immutable file (read-only)
         * 0x20 = Append only
         * 0x40 = File is not included in 'dump' command
         * 0x80 = Last accessed time should not update -- Frozen in time
         * ... reserved
         * 0x10000 = Hash indexed directory
         * 0x20000 = AFS directory
         * 0x40000 = Journal file data
         */
        ubyte4_t flags;
        /**
         * OS specific value.
         * Linux: reserved
         * HURD: "Translator"?
         * MASIX: reserved
         * 
         * Paranoia: reserved
         */
        ubyte4_t os1;
        ubyte4_t direct_pointers[12];
        ubyte4_t pointer_single_indirect;
        ubyte4_t pointer_double_indirect;
        ubyte4_t pointer_triple_indirect;
        // not sure what this does. "primarily used for nfs"?
        ubyte4_t generation_number;
        ubyte4_t extended_attribute_block;
        ubyte4_t upper_size;
        ubyte4_t fragment_address;
        /**
         * OS specific value.
         * 
         * Reference [https://wiki.osdev.org/Ext2#Inodes] for meanings on Linux, HURD, and MASIX.
         * 
         * Paranoia: reserved
         */
        ubyte4_t os2[3];
    } __attribute__((packed));
    enum DirectoryEntryType : ubyte1_t {
        UNKNOWN = 0,
        FILE,
        DIRECTORY,
        DEV_CHAR,
        DEV_BLOCK,
        FIFO,
        SOCKET,
        LINK_SOFT
    };
    struct DirectoryMetadata {
        ubyte4_t inode;
        ubyte2_t total_size;
        ubyte1_t name_len;
        DirectoryEntryType type;
    } __attribute__((packed));
    struct DirectoryEntry {
        DirectoryMetadata metadata;
        // maximum length is literally just 256 cuz i'm too lazy to make a large name implementation
        char name[];
    } __attribute__((packed));
    struct FilesystemInfo {
        Superblock* superblock;
        BlockGroupDescriptor* bgdt;
    };
}

namespace disk {
    union FSSpecific {
        void* generic;
        ext2::FilesystemInfo* ext2;
    };
    enum DriveType {
        ATA
    };
    struct Filesystem {
        char identifier[256];
        DriveType type;
        uint16_t index;
        uint16_t partition;
        FSSpecific fs;
        bool writable;
    };
}

namespace VFS {
    enum FilesystemType {
        ext2
    };
    union FSSpecificDetails {
        ext2::DirectoryMetadata ext2entry;
    };
    union FSSpecificFile {
        void* generic;
        ext2::Inode* ext2;
    };
    struct Filesystem {
        bool valid = true;
        disk::Filesystem* fs;
        FilesystemType type;
    };
    struct File {
        bool valid = false;
        fsid_t filesystem;
        fid_t file;
        FSSpecificDetails details;
    };
    struct OpenFile {
        bool valid = true;
        File file;
        FSSpecificFile data;
    };
}

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

enum PS2Returns {
    NoPS2Controller,
    PS2Timeout,
    SelfTestFailure,
    NoWorkingPorts,
    Success,
    NoAck,
    UnexpectedState,
};

enum PS2_STATES {
    NoProcess,
    WaitingForAck,
    Acked,
    EnablingScanning,
    WaitingForScancodes,
    EatingScancode,
    WaitingForPart,
    Failure,
    Unimplemented,
};

struct ps2stateMachine {
    PS2_STATES state = NoProcess;
    uint8_t lastScancode = 0; // formerly queueSize
    uint8_t data[64] = {};
    uint64_t currentData = 0;
    uint8_t firstScancode = 0;
    uint8_t flags = 0;
    uint8_t scancodeSet = 0x01;
    bool overwhelmed = 0;
    bool ready = 1;
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

struct KeyAction {
    bool initialized = false;
    KeyCode code;
    KeyMode mode;
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
    exit = 0x00,
    info,
    askNice,
    elevateU,
    elevateK,
    start,
    nice,
    kill,
    tell,
    hear,
    trace,
    setgroup,
    malloc = 0x10,
    free,
    mperm,
    mgetperms,
    yield = 0x20,
    sleep,
    fopen = 0x30,
    fmode,
    read,
    write,
    finfo,
    mkdir = 0x35,
    sync,
    fclose,
    RESERVED_34,
    unlink,
    sym,
    hard,
    seek,
    chperm,
    sinceboot = 0x60,
    beep,
    kinfo = 0x70,
    rinfo
    // SCALL_REQKERNEL = 100,
    // SCALL_REQELEVATE = 101,
    // SCALL_BOOSTTRUST = 102,
    // SCALL_EXIT = 200,
    // SCALL_PLACE = 201,
    // SCALL_WAITPLACED = 202,
    // SCALL_KILL = 203, // RE
    // SCALL_ME = 204,
    // SCALL_WHO = 205,
    // SCALL_CHANGERUDE = 206,
    // SCALL_SIGNAL = 207,
    // SCALL_FETCHFILE = 300,
    // SCALL_SETINTENT = 301,
    // SCALL_READ = 302,
    // SCALL_REPLACE = 303, // EC
    // SCALL_APPEND = 304, // EC
    // SCALL_DELETEFILE = 305, // RE
    // SCALL_CREATEFILE = 306,
    // SCALL_COPY = 307,
    // SCALL_MOUNTFS = 308,
    // SCALL_EJECTFS = 309, // RE
    // SCALL_FSINFO = 310,
    // SCALL_SLEEP = 400,
    // SCALL_GETFAIL = 401,
    // SCALL_YIELD = 402,
    // SCALL_NETDEAD = 500,
    // SCALL_NETCON = 501,
    // SCALL_NETKILL = 502,
    // SCALL_NETBIND = 503, // EC
    // SCALL_NETACCEPT = 504,
    // SCALL_NETSEND = 505,
    // SCALL_NETRECV = 506,
    // SCALL_NETINFO = 507,
    // SCALL_DNSRES = 508,
    // SCALL_DNSLOCAL = 509,
    // SCALL_NETFIREWALL = 600, // RE
    // SCALL_NETMONITOR = 601, // RE
    // SCALL_GETNETMONITOR = 602, // RE
    // SCALL_SETFILEPERM = 603, // RE
    // SCALL_GETFILEPERM = 604,
    // SCALL_MALLOC = 700,
    // SCALL_MFREE = 701,
    // SCALL_MINFO = 702,
    // SCALL_LISTDEVICES = 800,
    // SCALL_UPTIME = 900,
    // SCALL_SHUTDOWN = 901, // EC
    // SCALL_RESTART = 902, // EC
    // SCALL_SUSPEND = 903 // EC
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

struct RSDPLegacyDescriptor {
	char signature[8];		// "RSD PTR "
	uint8_t checksum;		// Entire struct must sum to 0
	char oem_id[6];
	uint8_t revision;		// 0 = ACPI 1.0
	uint32_t rsdt_address;	// 32-bit physical address of RSDT
} __attribute__((packed));

struct RSDPModernDescriptor {
	char signature[8];		// "RSD PTR "
	uint8_t checksum;		// Entire struct must sum to 0
	char oem_id[6];
	uint8_t revision;		// 0 = ACPI 1.0
	uint32_t rsdt_address;	// 32-bit physical address of RSDT

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct ACPISTDHeader {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEM_id[6];
    char OEM_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct RSDTDescriptor {
    ACPISTDHeader header;
    uint32_t other_std[]; // variable length array of table pointers
} __attribute__((packed));

struct XSDTDescriptor {
    ACPISTDHeader header;
    uint64_t other_std[]; // variable length array of table pointers
} __attribute__((packed));

struct ACPITables {
    bool isValid;
    ACPISTDHeader header;
    size_t count;
    bool isXsdt;
    uint64_t* entries;
} __attribute__((packed));

struct ACPI_generic_register_position {
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
};

struct FADTTable {
    ACPISTDHeader header;
    uint32_t firmware_ctrl;
    uint32_t dstd;
    uint8_t reserved_0000; // used in acpi 1.0, for compatibility only. is ignored.
    uint8_t preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4_bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe_0_block;
    uint32_t gpe_1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe_0_length;
    uint8_t gpe_1_length;
    uint8_t gpe_1_base;
    uint8_t c_state_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    uint16_t boot_architecture_flags; // reserved in acpi 1.0, used since acpi 2.0

    uint8_t reserved_0001;
    uint32_t flags;

    ACPI_generic_register_position reset_reg;

    uint8_t reset_value;
    uint8_t reserved_0002[3];

    uint64_t x_firmwarecontrol;
    uint64_t x_dsdt;

    ACPI_generic_register_position x_pm1a_event_block;
    ACPI_generic_register_position x_pm1b_event_block;
    ACPI_generic_register_position x_pm1a_control_block;
    ACPI_generic_register_position x_pm1b_control_block;
    ACPI_generic_register_position x_pm2_control_block;
    ACPI_generic_register_position x_pm_timer_block;
    ACPI_generic_register_position x_gpe_0_block;
    ACPI_generic_register_position x_gpe_1_block;
};

// struct PageTableEntry {
//     uint32_t address12 : 20;
//     uint32_t avl : 3;
//     uint32_t global : 1 = 0;
//     uint32_t pat : 1 = 0;
//     uint32_t dirty : 1 = 0;
//     uint32_t accessed : 1 = 0;
//     uint32_t cacheDisable : 1 = 0;
//     uint32_t writeThrough : 1 = 0;
//     uint32_t userSupervisor : 1 = 0;
//     uint32_t writable : 1 = 0;
//     uint32_t present : 1 = 0;
// };

// struct PageTableEntry {
//     uint32_t present : 1 = 0;
//     uint32_t writable : 1 = 0;
//     uint32_t userSupervisor : 1 = 0;
//     uint32_t writeThrough : 1 = 0;
//     uint32_t cacheDisable : 1 = 0;
//     uint32_t accessed : 1 = 0;
//     uint32_t dirty : 1 = 0;
//     uint32_t pat : 1 = 0;
//     uint32_t global : 1 = 0;
//     uint32_t avl : 3;
//     uint32_t address12 : 20;
// };

typedef uint32_t PageTableEntry;

struct PageTable {
    PageTableEntry entries[1024];
};

typedef uint32_t PageDirectoryEntry;

struct PageDirectory {
    PageDirectoryEntry entries[1024] = {};
};

#define PTE_PRESENT   0x1
#define PTE_WRITABLE  0x2
#define PTE_USER      0x4
#define PDE_BIGPAGE   0x80

// struct PageDirectoryEntry {
//     // 1 if in physical memory.
//     uint32_t present : 1 = 0;
//     // 1 is read/write, 0 is read only.
//     uint32_t writable : 1 = 0;
//     // Always acessable by the Supervisor. If 1, it is also accessable by the user.
//     uint32_t userSupervisor : 1 = 0;
//     // If 1, write-through caching is enabled.
//     uint32_t writeThrough : 1 = 0;
//     // If 0, caching is enabled.
//     uint32_t cacheDisable : 1 = 0;
//     // If a PDE or PTE wqas read during translation, this bit is set. This will not be cleared automatically.
//     uint32_t accessed : 1 = 0;
//     // Available for usage by the OS
//     uint32_t avl1 : 1;
//     // Always 0.
//     uint32_t bigPage : 1 = 0;
//     // Available for usage by the OS
//     uint32_t avl : 4;
//     // Upper 20 bits of the page table address.
//     uint32_t address12 : 20;
// };

// 0000

// 0000, 0000, 00, 00, 00, 00
// 0000, ffff, 00, cf, 9a, 00
// 0000, ffff, 00, cf, 92, 00

// ??

// 0020, 2230, 00, 78, 6a, b5