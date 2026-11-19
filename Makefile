# Makefile for building a simple kernel with i686-elf-gcc

CONST = # Constants
STRIP ?= 0

# Paths to tools
AS = i686-elf-as
CC = i686-elf-g++
LD = i686-elf-gcc
OBJCP = i686-elf-objcopy
GRUB = grub-mkrescue
STRIPPER = strip

# Flags for the compiler and linker
CFLAGS = -ffreestanding -m32 -O2 -nostdlib -Wall -Wextra -fno-exceptions -fno-rtti -Isrc/
LDFLAGS = -T linker.ld -O2 -ffreestanding -nostdlib -lgcc -Isrc/

CFLAGS += $(CONST)

ifeq ($(STRIP),0)
	CFLAGS += -g
endif

# UACPI

UACPI_DIR = third_party/uacpi

CFLAGS += -I$(UACPI_DIR)/include

# Output files
KERNEL = bin/paranoia.bin
OBJ_FILES = data.o boot.o paranoia.o multiboot2.o terminal.o string.o utils.o serial.o pit.o math.o memory.o fail.o idt.o font.o idtroutine.o gdt.o isrs.o syscall.o driver_framebuf.o driver_mbr.o driver_ps2ctl.o driver_disk_ata.o driver_disk_generic.o driver_fs_ext2.o driver_vfs.o driver_lex.o driverman.o pic.o acpi.o page.o scheduler.o
ISO_IMAGE = paranoia.iso

ISO_DIR = iso

# # uACPI configuration
# OBJ_FILES += $(UACPI_OBJ)

# UACPI_DIR = third_party/uacpi
# UACPI_CC = i386-elf-gcc

# UACPI_SRC = $(shell find $(UACPI_DIR)/source -name '*.c')
# UACPI_OBJ = $(patsubst $(UACPI_DIR)/%.c,bin/uacpi/%.o,$(UACPI_SRC))

# UACPI_CFLAGS = -ffreestanding -m32 -nostdlib -O2 \
#                -Wall -Wextra -fno-builtin \
#                -I$(UACPI_DIR)/include

# # Compile uACPI
# bin/uacpi/%.o: $(UACPI_DIR)/%.c
# 	mkdir -p $(dir $@)
# 	$(UACPI_CC) $(UACPI_CFLAGS) -c $< -o $@

# CFLAGS += -I$(UACPI_DIR)/include

# The default target
all: $(ISO_IMAGE)

boot.o: src/boot.s
	$(AS) $< -o $@

idtroutine.o: src/idtroutine.s
	$(AS) $< -o $@

isras.o: src/isr.s
	$(AS) $< -o $@

# Compile the C code (with inline assembly)
multiboot2.o: src/multiboot2.cpp
	$(CC) $(CFLAGS) -c $< -o $@

serial.o: src/serial.cpp
	$(CC) $(CFLAGS) -c $< -o $@

data.o: src/data.cpp
	$(CC) $(CFLAGS) -c $< -o $@

paranoia.o: src/paranoia.cpp
	$(CC) $(CFLAGS) -c $< -o $@

terminal.o: src/terminal.cpp
	$(CC) $(CFLAGS) -c $< -o $@

string.o: src/string.cpp
	$(CC) $(CFLAGS) -c $< -o $@

math.o: src/math.cpp
	$(CC) $(CFLAGS) -c $< -o $@

font.o: src/font.cpp
	$(CC) $(CFLAGS) -c $< -o $@

memory.o: src/memory.cpp
	$(CC) $(CFLAGS) -c $< -o $@

utils.o: src/utils.cpp
	$(CC) $(CFLAGS) -c $< -o $@

pit.o: src/pit.cpp
	$(CC) $(CFLAGS) -c $< -o $@

fail.o: src/fail.cpp
	$(CC) $(CFLAGS) -c $< -o $@

idt.o: src/idt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

isrs.o: src/isrs.cpp
	$(CC) $(CFLAGS) -mgeneral-regs-only -c $< -o $@

gdt.o: src/gdt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

syscall.o: src/syscall.cpp
	$(CC) $(CFLAGS) -c $< -o $@

pic.o: src/pic.cpp
	$(CC) $(CFLAGS) -c $< -o $@

acpi.o: src/acpi.cpp
	$(CC) $(CFLAGS) -c $< -o $@

page.o: src/page.cpp
	$(CC) $(CFLAGS) -c $< -o $@

sched.o: src/sched.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driverman.o: src/drivers/driverman.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_mbr.o: src/drivers/mbr.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_video.o: src/drivers/video.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_ps2ctl.o: src/drivers/ps2.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_disk_ata.o: src/drivers/ata.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_disk_generic.o: src/drivers/genericdisk.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_fs_ext2.o: src/drivers/ext2.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_vfs.o: src/drivers/vfs.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_lex.o: src/drivers/lex.cpp
	$(CC) $(CFLAGS) -c $< -o $@

driver_framebuf.o: src/drivers/framebuf.cpp
	$(CC) $(CFLAGS) -c $< -o $@

scheduler.o: src/scheduler.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link the kernel
$(KERNEL): $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $(KERNEL) $(OBJ_FILES)
ifeq ($(STRIP),2)
	$(STRIPPER) --strip-all bin/paranoia.bin
endif

	# Make the kernel bootable by adding the multiboot header and padding it to a 512-byte boundary
	$(OBJCP) --pad-to=512 $(KERNEL)

$(ISO_IMAGE): $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	cd iso
	$(GRUB) -o $(ISO_IMAGE) $(ISO_DIR)
	cd ..

# Clean the generated files
clean:
	rm -f $(OBJ_FILES) $(KERNEL)