# Cross-compiler tools
CC=i686-elf-gcc
LD=i686-elf-ld
AS=i686-elf-as
GRUB_MKRESCUE=grub-mkrescue

# Source and output files
SRC_DIR=src
BUILD_DIR=build
ISO_DIR=iso
KERNEL=$(BUILD_DIR)/kernel.bin

# Compiler and linker flags
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -g
LDFLAGS=-T linker.ld -nostdlib

# All target
all: $(KERNEL) iso

# Use compile multiboot.asm to multiboot.o
$(BUILD_DIR)/multiboot.o: $(SRC_DIR)/multiboot.asm
	mkdir -p $(BUILD_DIR)
	nasm -f elf32 src/multiboot.asm -o build/multiboot.o
# $(CC) $(CFLAGS) -c $< -o $@

# Compile kernel.c to object file
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


# Link kernel binary
$(KERNEL): $(BUILD_DIR)/kernel.o $(BUILD_DIR)/multiboot.o
	$(LD) $(LDFLAGS) -o $@ $^

# Create ISO image
iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/
	cp $(ISO_DIR)/boot/grub.cfg $(ISO_DIR)/boot/grub/
	$(GRUB_MKRESCUE) -o $(BUILD_DIR)/uqaabOS.iso $(ISO_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)/boot/kernel.bin

