# Cross-compiler tools
CC=i686-elf-g++
LD=i686-elf-ld
AS=i686-elf-as
GRUB_MKRESCUE=grub-mkrescue

# Source and output files
SRC_DIR=src
BUILD_DIR=build
ISO_DIR=iso
# KERNEL=$(BUILD_DIR)/kernel.bin

# Compiler and linker flags
CXXFLAGS=-std=c++17 -g -ffreestanding -O2 -Wall -Wextra 
LDFLAGS=-T linker.ld -nostdlib   

# All target
all: $(KERNEL) iso

# Use compile multiboot.asm to multiboot.o
$(BUILD_DIR)/multiboot.o: $(SRC_DIR)/multiboot.asm
	mkdir -p $(BUILD_DIR)
	nasm -f elf32 src/multiboot.asm -o build/multiboot.o
# $(CC) $(CFLAGS) -c $< -o $@

# compile src/include/load_gdt.s to load_gdt.o
$(BUILD_DIR)/load_gdt.o: $(SRC_DIR)/include/load_gdt.asm 
	nasm -f elf32 $< -o $@

# Compile kernel.c to object file
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile gdt.cpp to object file
$(BUILD_DIR)/gdt.o: $(SRC_DIR)/include/gdt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o $(BUILD_DIR)/multiboot.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/load_gdt.o
	$(LD) $(LDFLAGS) -o $@ $^

# Create ISO image
iso: $(BUILD_DIR)/kernel.bin
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/
	cp $(ISO_DIR)/boot/grub.cfg $(ISO_DIR)/boot/grub/
	$(GRUB_MKRESCUE) -o $(BUILD_DIR)/uqaabOS.iso $(ISO_DIR)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR)/boot/kernel.bin

