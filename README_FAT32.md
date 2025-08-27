# FAT32 Filesystem Implementation

This document describes the FAT32 filesystem implementation in uqaabOS, with special focus on the enhanced file opening functionality.

## Enhanced File Opening

The `open` function in the FAT32 class has been enhanced to support opening files in subdirectories, not just in the root directory.

### Usage Examples

```cpp
// Open a file in the root directory (original functionality)
int fd1 = fat32.open("test.txt");

// Open a file in a subdirectory (new functionality)
int fd2 = fat32.open("/test/file.txt");

// Open a file in a nested subdirectory (new functionality)
int fd3 = fat32.open("/documents/reports/2023/report.txt");
```

### Path Format

The function supports both absolute paths (starting with '/') and relative paths (though relative paths are treated as absolute from the root).

- `/file.txt` - File in the root directory
- `/dir/file.txt` - File in a subdirectory
- `/dir1/dir2/file.txt` - File in a nested subdirectory

### Return Values

- Returns a file descriptor (non-negative integer) on success
- Returns -1 on failure (file not found, directory not found, path is a directory, etc.)

### Limitations

- Does not support relative paths from current directory (all paths are treated as absolute from root)
- Does not support paths with "." or ".." components
- Maximum path depth is limited by the filesystem structure

## Other Functions

The FAT32 implementation also includes:

- `ls(path)` - List files in a directory
- `mkdir(path)` - Create a new directory
- `touch(path)` - Create a new empty file
- `rm(path)` - Remove a file
- `rmdir(path)` - Remove a directory

All of these functions also support subdirectory paths in the same format as the `open` function.