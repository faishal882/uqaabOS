# Cross-compiler tools
CC=i686-elf-g++
LD=i686-elf-ld
AS=i686-elf-as
GRUB_MKRESCUE=grub-mkrescue

# Source and output files
SRC_DIR=src
BUILD_DIR=build
ISO_DIR=iso

# Compiler and linker flags
CFLAGS = -g -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nostartfiles -nodefaultlibs
LDFLAGS=-T linker.ld -nostdlib   

# All target
all: $(BUILD_DIR)/kernel.bin iso

# Use compile multiboot.asm to multiboot.o
$(BUILD_DIR)/multiboot.o: $(SRC_DIR)/multiboot.asm
	mkdir -p $(BUILD_DIR)
	nasm -f elf32 $< -o $@

# compile interruptstub.asm to interruptstub.o
$(BUILD_DIR)/interruptstub.o: $(SRC_DIR)/core/interrupts/interruptstub.asm
	nasm -f elf32 $< -o $@

# Compile kernel.cpp to object file
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile stdio.cpp to object file
$(BUILD_DIR)/stdio.o: $(SRC_DIR)/libc/stdio.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile string.cpp to object file
$(BUILD_DIR)/string.o: $(SRC_DIR)/libc/string.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile gdt.cpp to object file
$(BUILD_DIR)/gdt.o: $(SRC_DIR)/core/gdt.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile multitasking.cpp to object file
$(BUILD_DIR)/multitasking.o: $(SRC_DIR)/multitasking/multitasking.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile memorymanagement.cpp to object file
$(BUILD_DIR)/memorymanagement.o: $(SRC_DIR)/memorymanagement/memorymanagement.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile interrupts.cpp to object file
$(BUILD_DIR)/interrupts.o: $(SRC_DIR)/core/interrupts/interrupts.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile port.cpp to object file
$(BUILD_DIR)/port.o: $(SRC_DIR)/core/port.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile driver.cpp to object file
$(BUILD_DIR)/driver.o: $(SRC_DIR)/drivers/driver.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile pci.cpp to object file
$(BUILD_DIR)/pci.o: $(SRC_DIR)/drivers/pci.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile vga.cpp to object file
$(BUILD_DIR)/vga.o: $(SRC_DIR)/drivers/vga.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile keyboard.cpp to object file
$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/drivers/keyboard.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile mouse.cpp to object file
$(BUILD_DIR)/mouse.o: $(SRC_DIR)/drivers/mouse.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile ata.cpp to object file
$(BUILD_DIR)/ata.o: $(SRC_DIR)/drivers/storage/ata.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile msdospart.cpp to object file
$(BUILD_DIR)/msdospart.o: $(SRC_DIR)/filesystem/msdospart.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile fat32.cpp to object file
$(BUILD_DIR)/fat32.o: $(SRC_DIR)/filesystem/fat32.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
# Compile fat32_operations.cpp to object file
$(BUILD_DIR)/fat32_operations.o: $(SRC_DIR)/filesystem/fat32_operations.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile fat32_path_helpers.cpp to object file
$(BUILD_DIR)/fat32_path_helpers.o: $(SRC_DIR)/filesystem/fat32_path_helpers.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile fat32_write_helpers.cpp to object file
$(BUILD_DIR)/fat32_write_helpers.o: $(SRC_DIR)/filesystem/fat32_write_helpers.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.o $(BUILD_DIR)/multiboot.o \
                     $(BUILD_DIR)/gdt.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/string.o \
					 $(BUILD_DIR)/multitasking.o $(BUILD_DIR)/memorymanagement.o \
					 $(BUILD_DIR)/interrupts.o $(BUILD_DIR)/interruptstub.o $(BUILD_DIR)/port.o \
					 $(BUILD_DIR)/driver.o $(BUILD_DIR)/pci.o $(BUILD_DIR)/vga.o \
					 $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/mouse.o $(BUILD_DIR)/ata.o \
					 $(BUILD_DIR)/msdospart.o $(BUILD_DIR)/fat32.o $(BUILD_DIR)/fat32_operations.o \
					 $(BUILD_DIR)/fat32_path_helpers.o $(BUILD_DIR)/fat32_write_helpers.o

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