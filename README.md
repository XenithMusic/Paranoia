# PARANOIA
> Paranoia is a (to-be modular) kernel, that is extremely untrustworthy of programs.

## Features AND/OR Todo
- [x] Does not trust user-space programs in the slightest.
- [x] Memory Allocation (probably p bad, but it's good enough)
- [x] Keeping track of time poorly
- [X] Interrupt Descriptor Table
- [ ] Drivers
- [ ] Ext2 file system
- [ ] System Call API
- [ ] Executing other programs
- [ ] Scheduler and/or threading

## Dependencies
- grub
- [i686-elf-tools](https://wiki.osdev.org/GCC_Cross-Compiler)
## Compiling
```
~ chmod +x ./build
~ ./build
...
```
This will automatically build, and run with `qemu-system-i386`.

If you do not have `qemu-system-i386` installed, use whatever emulator you want to boot from the .iso file that can be found in the root directory.

Tested on:
- Linux Mint 21.3 (qemu-system-i386)