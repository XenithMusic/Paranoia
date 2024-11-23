# Makefile for building a simple kernel with i686-elf-gcc

CONST = # Constants

# Paths to tools
AS = i686-elf-as
CC = i686-elf-g++
LD = i686-elf-gcc
OBJCP = i686-elf-objcopy
GRUB = grub-mkrescue

# Flags for the compiler and linker
CFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

CFLAGS += $(CONST)

# Output files
KERNEL = bin/paranoia.bin
OBJ_FILES = boot.o paranoia.o terminal.o string.o assemblyUtils.o pit.o math.o memory.o
ISO_IMAGE = paranoia.iso

ISO_DIR = iso

# The default target
all: $(ISO_IMAGE)


boot.o: src/boot.s
	$(AS) $< -o $@

# Compile the C code (with inline assembly)
paranoia.o: src/paranoia.cpp
	$(CC) $(CFLAGS) -c $< -o $@

terminal.o: src/terminal.cpp
	$(CC) $(CFLAGS) -c $< -o $@

string.o: src/string.cpp
	$(CC) $(CFLAGS) -c $< -o $@

math.o: src/math.cpp
	$(CC) $(CFLAGS) -c $< -o $@

memory.o: src/memory.cpp
	$(CC) $(CFLAGS) -c $< -o $@

assemblyUtils.o: src/assemblyUtils.cpp
	$(CC) $(CFLAGS) -c $< -o $@

pit.o: src/pit.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link the kernel
$(KERNEL): $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $(KERNEL) $(OBJ_FILES)

	# Make the kernel bootable by adding the multiboot header and padding it to a 512-byte boundary
	$(OBJCP) --pad-to=512 $(KERNEL)

$(ISO_IMAGE): $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/

	$(GRUB) -o $(ISO_IMAGE) $(ISO_DIR)

# Clean the generated files
clean:
	rm -f $(OBJ_FILES) $(KERNEL)
	rm paranoia.iso