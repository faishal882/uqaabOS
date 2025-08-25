# FAT32 Filesystem Driver for uqaabOS

This document describes the FAT32 filesystem driver implementation for uqaabOS.

## Features

- Read-only FAT32 filesystem support
- 8.3 filename format support
- File reading and directory listing
- No dynamic memory allocation (uses stack/static buffers)
- Compatible with existing ATA PIO driver

## API

### Constructor
```cpp
FAT32(driver::ATA* disk, uint32_t partition_lba)
```
Initializes a FAT32 driver instance with the specified ATA device and partition LBA.

### initialize()
```cpp
bool initialize()
```
Reads and validates the BIOS Parameter Block (BPB) and calculates filesystem layout.

### open()
```cpp
int open(const char* path)
```
Opens a file and returns a file descriptor. Only supports root directory files with 8.3 names.

### read()
```cpp
int read(int fd, uint8_t* buf, uint32_t size)
```
Reads data from an open file.

### close()
```cpp
void close(int fd)
```
Closes an open file.

### list_root()
```cpp
void list_root()
```
Lists all files in the root directory.

## Usage Example

```cpp
uqaabOS::filesystem::FAT32 fat32(&ata_primary, partition_lba);
if (fat32.initialize()) {
    fat32.list_root();
    int fd = fat32.open("HELLO.TXT");
    if (fd > 0) {
        uint8_t buffer[512];
        int bytes = fat32.read(fd, buffer, 512);
        buffer[bytes] = '\0';
        libc::printf((char*)buffer);
        fat32.close(fd);
    }
}
```

## Implementation Details

The driver implements the following FAT32 structures:
- BIOS Parameter Block (BPB)
- Directory entries
- File allocation table traversal
- Cluster chaining

The implementation uses only stack and static buffers to avoid dynamic memory allocation.