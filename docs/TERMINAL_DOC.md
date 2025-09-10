# uqaabOS Terminal Documentation

## Table of Contents
1. [Overview](#overview)
2. [Available Commands](#available-commands)
3. [Command Details](#command-details)
4. [Path Format](#path-format)
5. [File System Features](#file-system-features)
6. [Technical Implementation](#technical-implementation)
7. [Error Handling](#error-handling)
8. [Future Enhancements](#future-enhancements)

## Overview

The uqaabOS terminal provides a command-line interface for interacting with the operating system, particularly the FAT32 filesystem. It supports standard file and directory operations, similar to Unix/Linux terminals.

## Available Commands

### 1. ls - List Directory Contents

Lists the contents of a directory.

**Syntax:**
```
ls [path]
```

**Parameters:**
- `path` (optional): The directory path to list. If omitted, lists the root directory.

**Examples:**
```
ls
ls /
ls /documents
ls /documents/reports
```

**Output:**
Displays files and directories in the specified path. Directories are marked with `[DIR]` and files show their size in bytes.

### 2. mkdir - Create Directory

Creates a new directory.

**Syntax:**
```
mkdir <path>
```

**Parameters:**
- `path`: The path of the directory to create. Can be in a subdirectory.

**Examples:**
```
mkdir /documents
mkdir /documents/reports
mkdir /test/newfolder
```

**Notes:**
- Creates all necessary parent directories if they don't exist.
- Shows an error if the directory already exists.

### 3. touch - Create Empty File

Creates a new empty file with allocated space.

**Syntax:**
```
touch <path>
```

**Parameters:**
- `path`: The path of the file to create.

**Examples:**
```
touch /test.txt
touch /documents/file.txt
touch /documents/reports/2023/report.txt
```

**Features:**
- Immediately allocates at least one cluster for the file.
- Initializes the allocated cluster with zeros.
- Files can be written to immediately after creation.

### 4. rm - Remove File

Deletes a file from the filesystem.

**Syntax:**
```
rm <path>
```

**Parameters:**
- `path`: The path of the file to delete.

**Examples:**
```
rm /test.txt
rm /documents/file.txt
rm /documents/reports/2023/report.txt
```

**Notes:**
- Only works on files, not directories.
- Frees all clusters allocated to the file.
- Shows an error if the path is a directory.

### 5. rmdir - Remove Directory

Deletes a directory and its contents.

**Syntax:**
```
rmdir <path>
```

**Parameters:**
- `path`: The path of the directory to delete.

**Examples:**
```
rmdir /documents
rmdir /test/folder
```

**Features:**
- Recursively deletes all contents of the directory (files and subdirectories).
- Frees all clusters allocated to the directory and its contents.
- Shows an error if the path is a file.

### 6. cat - Display File Contents

Displays the contents of a file.

**Syntax:**
```
cat <path>
```

**Parameters:**
- `path`: The path of the file to display.

**Examples:**
```
cat test.txt          # File in root directory
cat /test.txt         # File in root directory (explicit)
cat /documents/file.txt
```

**Notes:**
- Opens the file, reads its contents, and displays them to the terminal.
- Works with files of any size (reads in chunks).
- Automatically handles both relative paths (e.g., "test.txt") and absolute paths (e.g., "/test.txt") for root directory files.
- Fixed buffer overflow issues that could cause "bound range exceeding exception".

### 7. echo - Display Text

Displays text to the terminal.

**Syntax:**
```
echo <text>
```

**Parameters:**
- `text`: The text to display.

**Examples:**
```
echo Hello, World!
echo This is a test message
echo Current directory contents:
```

### 8. clear - Clear Screen

Clears the terminal screen.

**Syntax:**
```
clear
```

**Notes:**
- Properly clears the screen buffer and resets the cursor position.

### 9. help - Show Help

Displays information about available commands.

**Syntax:**
```
help
```

## Path Format

All commands support paths in the following formats:

1. **Absolute paths** (starting with '/'):
   - `/file.txt` - File in the root directory
   - `/dir/file.txt` - File in a subdirectory
   - `/dir1/dir2/file.txt` - File in a nested subdirectory

2. **Relative paths** (for root directory files):
   - `file.txt` - File in the root directory (automatically converted to `/file.txt`)

3. **Root directory** (special cases):
   - `/` - Root directory
   - `.` - Current directory (treated as root)
   - `..` - Parent directory (treated as root since we don't track current directory)

## File System Features

### Enhanced File Opening
The `open` function supports opening files in subdirectories, not just in the root directory.

### Cluster Allocation
Files created with `touch` have at least one cluster allocated to them, ensuring they can be written to immediately after creation.

### Write Support
The filesystem supports writing data to files through the `write` function, which:
- Automatically allocates clusters as needed when extending files
- Maintains proper file metadata (size, cluster chain)
- Handles files with and without pre-allocated clusters

## Technical Implementation

### Terminal Components
1. **Terminal Class**: Main terminal implementation
2. **Keyboard Event Handler**: Processes keyboard input
3. **Command Parser**: Parses user input into commands and arguments
4. **Command Executors**: Individual functions for each command

### Input Handling
- Processes characters one at a time as they are typed
- Supports backspace for editing
- Executes commands when Enter is pressed
- Now supports the '/' character for absolute paths

### Buffer Management
- Input buffer size: 256 characters
- Maximum arguments per command: 32
- Automatic buffer clearing after command execution
- Fixed buffer overflow issues in the `cat` command to prevent "bound range exceeding exception"

### Screen Clearing
- Uses proper VGA text mode clearing instead of printing blank lines
- Resets cursor position to top-left corner
- Updates hardware cursor position

### Path Handling for Root Directory Files
The terminal automatically handles both relative and absolute paths for files in the root directory:
- When a user types `cat test.txt`, the terminal automatically converts it to `/test.txt`
- This ensures proper file lookup in the root directory
- Works for all file operations (cat, rm, etc.)

## Error Handling

The terminal and underlying filesystem include comprehensive error handling:
- Invalid paths
- Non-existent files/directories
- Permission issues (trying to open directories as files)
- Disk read/write errors
- Memory allocation failures
- Buffer overflow protection in the `cat` command

## Future Enhancements

Possible future improvements to the terminal:
1. Command history (using up/down arrow keys)
2. Tab completion for file/directory names
3. Proper screen clearing implementation
4. Current directory tracking
5. Additional commands (cp, mv, pwd, etc.)
6. Command piping and redirection
7. Environment variables
8. Command aliases