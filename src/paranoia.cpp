#include <stdint.h>
#include "terminal.h"
#include "syscall.h"
#include "string.h"
#include "memory.h"
#include "const.h"
#include "types.h"
#include "fail.h"
#include "acpi.h"
#include "page.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "pit.h"

// DRIVERS

#include "drivers/driverman.h"
#include "drivers/genericdisk.h"
#include "drivers/ext2.h"
#include "drivers/ata.h"
#include "drivers/vfs.h"
#include "vector.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

/*

TODO:
- Memory Management

    [X] Basic kmalloc
        [X] Allocate
    [X] Basic kfree

- Interrupt Descriptor Table
    [X] Make space for the IDT
    [X] Tell the CPU where that is
    [X] Tell the PIC to screw off with BIOS defaults (https://wiki.osdev.org/PIC#Programming_the_PIC_chips)
    [X] Write a couple of ISR (interrupt service routines) for IRQs and exceptions
            -- i have exceptions right now --
    [X] Put the addresses of the ISR handlers in the appropiate descriptors (in the IDT)

- PS/2 Keyboard Driver
    BUG: Changing scancode sets doesn't work
    TODO: Verify that the PS/2 Controller exists.

*/

extern "C" {

    char* str; // pointer if anything demands a pointer
    multiboot_info_t* g_mbi;
    IDTEntry* g_idt;
    uint8_t k_stack_new[8192];
    uint8_t u_stack_demo[8192];
    ACPITables rsdt;
    TSS32 tss = {0};

    void GDTinitialized(IDTEntry* idt,multiboot_info_t* mbi);    
    void PagingInitialized(IDTEntry* idt,multiboot_info_t* mbi);
    void kernel_main(multiboot_info_t* mbi) {
        // Initialize GDT as the VERY FIRST THING YOU DO.
        GDTEntry* gdt = (GDTEntry*)0x1000; // Define GDT pointer at a fixed memory location
        tss.esp0 = (uint32_t)k_stack_new;
        GDTPointer gdtpointer = initGDT(gdt,&tss);
        __attribute__((aligned(0x10))) 
        IDTEntry* idt = (IDTEntry*)((uint32_t)gdt + gdtpointer.limit + 1); // Define IDT pointer immediately following the GDT
        // Now that we have a GDT, we can continue kernel initialization.
        GDTinitialized(idt,mbi);
    }
    void GDTinitialized(IDTEntry* idt,multiboot_info_t* mbi) {
        Paging::initwkp(); // Initialize Paging as the VERY SECOND THING YOU DO.
        PagingInitialized(idt,mbi);
    }
    void PagingInitialized(IDTEntry* idt,multiboot_info_t* mbi) {
        // Set up basic environment (screen, interrupts, etc.)

        Allocator::init();

        void* phys_page = Paging::kalloc4K();
        if (phys_page == nullptr) {
            fault(-103,"page is null","paranoia.PagingInitialized");
        }
        if ((uintptr_t)phys_page&0xFFF) {
            fault(-103,"page isnt aligned","paranoia.PagingInitialized");
        }
        Paging::mapArbitraryRange(&Paging::kernelPD,0x400000,(uint32_t)phys_page,80,0x7);
        uint8_t* test = (uint8_t*)phys_page;
        test[0] = 0xAA;
        test[0xFFF] = 0x55;
        if (test[0] != 0xAA or test[0xFFF] != 0x55) {
            fault(-103,"page did not write","paranoia.PagingInitialized");
        }

        // Backlog of pre-terminal init
        Terminal::init();
        Terminal::setCursorConfig((0xE << 4) | 0xF);
        Terminal::enableCursor();
        Terminal::print("\n:: CPU INIT\n\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" GDT initialized.");
            Terminal::print("\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" Allocator initialized.");
            Terminal::print("\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" Terminal initialized.");
            Terminal::print("\n");

        // #warning "The RSDT is being replaced with NULL; VERY dangerous, and WILL result in unintended behavior."
        // #warning "If you see this while compiling, PLEASE REPORT THIS AS A BUG."

        set_pit_count(0);

        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" PIT reset, time is measured in seconds since this message.");
            Terminal::print("\n");

        rsdt = find_rsdt();
        // ACPITables rsdt = {};

        // if (rsdt.isValid == false) {
        //     if (rsdt.count == 0x00) {
        //         fault(-500,"Fail 0x00");
        //     } else {
        //         fault(-500,"Unknown fail.");
        //     }
        // } else {
        //     Terminal::print("RSDT is valid.");
        // }

        // INTERRUPTS

        Terminal::print("\n:: INTERRUPTS (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");
        
        initIDT(idt);

        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] IDT initialized.");
            Terminal::print("\n");
        
        g_idt = idt;
        g_mbi = mbi;

        __asm__ __volatile__ (
            "mov %0, %%esp\n"
            "xor %%ebp, %%ebp\n"
            "call PostRestack\n"
            : : "r"(k_stack_new) : "memory"
        );
    }
    void PostRestack() {
        // if (!(g_mbi->flags & (1<<12))) {
        //     fault(-1000,"Expected a framebuffer.","PostRestack");
        // }
        Terminal::print("\n:: DRIVERS (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");
        
        drivermanager::init(&rsdt,g_mbi);

        // START OF FS TESTING STUFF !!!

        // Terminal::print("mounting\n");
        // disk::Filesystem* primary = ext2::mount(disk::DriveType::ATA,disk_ata::ATADrive::MASTER,0);
        // if (primary == nullptr) {
        //     fault(-801,"Invalid filesystem.");
        // } else {
        //     Terminal::print("valid filesystem!\n");
        // }
        // Terminal::print("This drive has been mounted ");
        //     Terminal::print(parseInt(primary->fs.ext2->superblock->mounts_since_fsck,10));
        //     Terminal::print(" times!\n");
        // Terminal::print("This drive should fsck after ");
        //     Terminal::print(parseInt(primary->fs.ext2->superblock->mounts_to_fsck,10));
        //     Terminal::print(" mounts.\n");
        // Terminal::print("testing inode read\n");
        // ext2::Inode* inode = ext2::open_inode(primary,2);
        // if (inode == nullptr) {
        //     Terminal::print("root is nullptr\n");
        //     while(1){}
        // }
        // char* name = "abcdefg.hijklmn\0";
        // name = "demo";
        // Pair<bool,ext2::DirectoryMetadata> entry = ext2::get_child(primary,inode,name);
        // if (entry.first == false) {
        //     Terminal::print("nonexistant child 'demo'??\n");
        // } else {
        //     Terminal::print("inode id is ");
        //         Terminal::print(parseInt(entry.second.inode,10));
        //         Terminal::print("\n");
        //     ext2::close_inode(inode);
        //     inode = ext2::open_inode(primary,entry.second.inode);
        //     if (inode == nullptr) {
        //         Terminal::print("demo is nullptr\n");
        //         while(1){}
        //     }
        //     name = "file.txt";
        //     entry = ext2::get_child(primary,inode,name);
        //     if (entry.first == false) {
        //         Terminal::print("nonexistant child 'demo/file.txt'??\n");
        //     } else {
        //         ext2::close_inode(inode);
        //         inode = ext2::open_inode(primary,entry.second.inode);
        //         if (inode == nullptr) {
        //             Terminal::print("file is nullptr");
        //             while(1){}
        //         }
        //         uint8_t buffer[26];
        //         kmemset(buffer,0,26);
        //         ext2::read_inode(primary,inode,0,20,buffer);
        //         Terminal::print("Read from file.txt: ");
        //             Terminal::print((char*)buffer);
        //             Terminal::print("\n");
        //     }
        // }
        // Terminal::print("unmounting\n");
        // ext2::unmount(primary);
        // Terminal::print("ext2 mount test successful\n");


        // Terminal::print("vector test\n");
        // Vector<char*> test;
        // test.open();
        // for (int i=0;i<=2040;i++) {
        //     test.add("a");
        // }
        // test.add("b");
        // if (test[2040][0] != 'a') fault(1000,parseInt(test[2040][0],10));
        // if (test[2041][0] != 'b') fault(1001,parseInt(test[2041][0],10));
        // test.close();
        // Terminal::print("test succeeded\n");

        // Terminal::print("vfs test\n");
        // VFS::init();
        // Terminal::print("mounting\n");
        // fsid_t primary2 = VFS::mount(VFS::FilesystemType::ext2,disk::DriveType::ATA,disk_ata::ATADrive::MASTER,
        //     0,"primary");
        // Terminal::print("getting file /primary/demo/file.txt\n");
        // VFS::File file = VFS::getFileFromPath("/primary/demo/file.txt");
        // if (file.valid) {
        //     Terminal::print("valid! inode at ");
        //         Terminal::print(parseInt(file.details.ext2entry.inode,10));
        //         Terminal::print("\n");
        // }
        // Terminal::print("opening file\n");
        // fid_t fid = VFS::openFile(file);
        // Terminal::print("reading file\n");
        // char anotherBuffer[260];
        // kmemset(anotherBuffer,0,260);
        // VFS::readFile(fid,0,200,anotherBuffer);
        // Terminal::print("- i read in '");
        //     Terminal::print(anotherBuffer);
        //     Terminal::print("'!\n");
        // Terminal::print("closing file\n");
        // VFS::closeFile(fid);
        // Terminal::print("unmounting\n");
        // VFS::unmount(primary2);
        // Terminal::print("vfs test succeeded\n");
        // 
        // write test
        // (this succeeded last time i checked)
        // (this also writes across a section boundary, which was why it broke previously. fixed that bug tho)
        //
        // char* test = "abcdefghijklmnopqrstuvwxyz\0\0\0\0\0\0";
        // Terminal::print(test);
        //     Terminal::print("\n\n");
        // disk::write_bytes(disk_ata::ATADrive::MASTER,0,500,26,test);
        // kmemset(test,0,26);
        // Terminal::print(test);
        //     Terminal::print("\n\n");
        // busy_sleep(1);
        // disk::get_bytes(disk_ata::ATADrive::MASTER,0,500,32,test);
        // Terminal::print(test);
        //     Terminal::print("\n\n");
        while (1){}

        Terminal::print("\n:: SERVICES (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");
        
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] Scheduler initialized.");
            Terminal::print("\n");
        // ext2::Superblock* superblock;
        // ext2::get_superblock(superblock);
        // ext2::BlockGroupDescriptor bgd_table[superblock->total_blocks / superblock->group_blocks + 1];
        // ext2::read_bgd_table(bgd_table,superblock);
        // Pair<ext2::ExtState, ubyte4_t> src = ext2::find_path("/sys/init.bin",superblock,bgd_table);
        // ext2::Inode* src_inode;
        // ext2::get_inode(src_inode,src.second,superblock,bgd_table);
        // char buffer[4096];
        // ext2::read_block(0,4096,buffer);
        // Terminal::print(buffer);
        Terminal::print("\n[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] Found init executable.");
            Terminal::print("\n");
        // process::load_bin((void*)0x400000,src_inode,128);
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] Created init process.");
            Terminal::print("\n");

        // no services yet.
        // - init syscalls
        // - init scheduler
        // - create first task and notify the scheduler

        Terminal::print("System information:\n");
        Terminal::print("KERNEL:               PARANOIA\n");
        Terminal::print("- VERSION:            ");
            Terminal::print(CONST_VERSION);
            Terminal::print("\n");
        Terminal::print("- COMPILATION DATE:   ");
            Terminal::print(CONST_COMPDATE);
            Terminal::print("\n");
        Terminal::print("- AUTHORED BY:        ");
            Terminal::print(CONST_AUTHOR);
            Terminal::print("\n");
        Terminal::print("- KERNEL SIZE:        ");
            Terminal::print(parseDouble((double)CONST_KERNELSIZE/4/1024,str,2));
            Terminal::print("MiB\n");
        if (CONST_DEBUGGING) {
            Terminal::print("!! WARNING: Running in debug mode, output may be far more verbose than usual!\n");
        }
        
        // // You can add more setup here (keyboard, time, etc.)

        // // approx. 0.0000326 seconds per print instruction
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" first call\n");
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" second call\n");
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" third call\n");
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" fourth call\n");

        Terminal::print("\n:: HANDOFF (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");

        #ifdef CONST_DEBUGGING
        if (Paging::is_paging_enabled())
            Terminal::print("[DEBUG CHECK] Paging is enabled.\n");
        else {
            Terminal::print("[DEBUG CHECK] Paging is disabled.\n");
        }

        Terminal::printdebug("Testing near memory...\n");
        char* character = (char*)0x10000;
        character[0] = 'A';
        if (*character == 'A')
            Terminal::printdebug("[DEBUG CHECK] Succeeded.\n");

        // Terminal::printdebug("Testing far memory...\n");
        // character = (char*)0x1000000;
        // character[0] = 'A';
        // if (*character == 'A')
        //     Terminal::printdebug("[DEBUG CHECK] Succeeded. (PAGING IS DISABLED)\n");
        #endif

        double lastTick;
        double tick;

        // temporary busy-loop. handoff to the scheduler here.
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] Entering scheduler!");
            Terminal::print("\n");

        // while (1) {
        //     tick = get_pit_seconds();
        //     if (lastTick+0.01 < tick) {
        //         lastTick = tick;
        //         if (ps2keyboard::keysDown[KeyCode::MODIF_ANY_CONTROL] and 
        //             ps2keyboard::keysDown[KeyCode::MODIF_ANY_SHIFT] and 
        //             not ps2keyboard::keysDown[KeyCode::MODIF_ANY_ALT] and 
        //             not ps2keyboard::keysDown[KeyCode::MODIF_ANY_META] and 
        //             ps2keyboard::keysDown[KeyCode::ALPHA_T]) {
        //             Terminal::print("[");
        //                 Terminal::print(parseDouble(tick,str,10));
        //                 Terminal::print("] Keybind message print.");
        //                 Terminal::print("\n");
        //         } else {
        //             const Pair<KeyCode, char>* list = codesAscii;
        //             if (ps2keyboard::keysDown[KeyCode::MODIF_ANY_SHIFT]) {
        //                 for (const Pair<KeyCode,char> pair : codesAsciiCapital) {
        //                     if (ps2keyboard::keysDown[pair.first]) {
        //                         ps2keyboard::keysDown[pair.first] = false;
        //                         Terminal::print(&pair.second);
        //                     }
        //                 }
        //             } else {
        //                 for (const Pair<KeyCode,char> pair : codesAscii) {
        //                     if (ps2keyboard::keysDown[pair.first]) {
        //                         ps2keyboard::keysDown[pair.first] = false;
        //                         Terminal::print(&pair.second);
        //                     }
        //                 }
        //             }
        //         }
        //         ps2keyboard::processScancodes();
        //     }
        //     // if (ps2keyboard::state.state == EatingScancode)
        //     //     ps2keyboard::processScancodes();
        //     // The OS runs here
        //     // Terminal::swapBuffers();
        // }
        busy_sleep(5.0);
        
        // TODO: Somehow, I need to shut down the computer. In the meantime, I'm just gonna print and halt.
        Terminal::clearScreen();
        Terminal::print("\n\n\n\n\n\n\n\nHi!\n");
        Terminal::print("It is really really hard to shut down the computer.\n");
        Terminal::print("I am not joking, I need an AML interpreter, which is painful.\n");
        Terminal::print("I may implement uACPI at some point, however I have not yet done so.\n");
        Terminal::print("(https://uacpi.github.io)\n\n");
        Terminal::print("Sorry for the inconvenience.\n\n");
        Terminal::print("Please manually turn off the computer.\n\n");
        Terminal::print("Note: If you are using QEMU, Bochs, Virtualbox, or Cloud Hypervisor,\n");
        Terminal::print("  your computer will shut down without intervention in 5 seconds.\n");
        busy_sleep(5.0);
        Power::shutdown();
        while(1){
            induceHalt();
        }
    }
};