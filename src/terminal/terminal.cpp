#include "../include/terminal/terminal.h"

namespace uqaabOS {
namespace terminal {

Terminal::Terminal(filesystem::FAT32* fat32_instance) {
    this->fat32 = fat32_instance;
    this->buffer_position = 0;
    for (int i = 0; i < TERMINAL_BUFFER_SIZE; i++) {
        this->input_buffer[i] = '\0';
    }
}

void Terminal::initialize() {
    // Clear the screen when terminal starts
    libc::clear_screen();
    
    libc::printf("Terminal initialized. Type 'help' for available commands.\n");
    print_prompt();
}

void Terminal::print_prompt() {
    libc::printf("$ ");
}

void Terminal::handle_key_press(char c) {
    // Handle Enter key
    if (c == '\n' || c == '\r') {
        libc::printf("\n");
        if (buffer_position > 0) {
            input_buffer[buffer_position] = '\0';
            execute_command(input_buffer);
            buffer_position = 0;
            for (int i = 0; i < TERMINAL_BUFFER_SIZE; i++) {
                input_buffer[i] = '\0';
            }
        }
        print_prompt();
    }
    // Handle Backspace
    else if (c == '\b') {
        if (buffer_position > 0) {
            buffer_position--;
            input_buffer[buffer_position] = '\0';
            // Simple backspace output (in a real terminal you might want to move cursor)
            libc::printf("\b \b");
        }
    }
    // Handle regular characters
    else if (buffer_position < TERMINAL_BUFFER_SIZE - 1) {
        input_buffer[buffer_position] = c;
        buffer_position++;
        libc::putchar(c);
    }
}

int Terminal::parse_command(char* command, char* argv[]) {
    int argc = 0;
    char* token = command;
    int i = 0;
    
    // Skip leading spaces
    while (*token == ' ') token++;
    
    while (*token != '\0' && argc < MAX_ARGS - 1) {
        argv[argc] = token;
        argc++;
        
        // Find end of token
        while (*token != ' ' && *token != '\0') token++;
        
        // If we found a space, terminate the token and move to next
        if (*token == ' ') {
            *token = '\0';
            token++;
            // Skip additional spaces
            while (*token == ' ') token++;
        }
    }
    
    argv[argc] = nullptr;
    return argc;
}

void Terminal::execute_command(char* command) {
    char* argv[MAX_ARGS];
    int argc = parse_command(command, argv);
    
    if (argc == 0) {
        return;
    }
    
    if (libc::strcmp(argv[0], "ls") == 0) {
        handle_ls(argc, argv);
    } else if (libc::strcmp(argv[0], "mkdir") == 0) {
        handle_mkdir(argc, argv);
    } else if (libc::strcmp(argv[0], "touch") == 0) {
        handle_touch(argc, argv);
    } else if (libc::strcmp(argv[0], "rm") == 0) {
        handle_rm(argc, argv);
    } else if (libc::strcmp(argv[0], "rmdir") == 0) {
        handle_rmdir(argc, argv);
    } else if (libc::strcmp(argv[0], "cat") == 0) {
        handle_cat(argc, argv);
    } else if (libc::strcmp(argv[0], "echo") == 0) {
        handle_echo(argc, argv);
    } else if (libc::strcmp(argv[0], "help") == 0) {
        handle_help();
    } else if (libc::strcmp(argv[0], "clear") == 0) {
        handle_clear();
    } else if (libc::strcmp(argv[0], "") == 0) {
        // Empty command, do nothing
    } else {
        libc::printf("Command not found: ");
        libc::printf(argv[0]);
        libc::printf("\n");
    }
}

void Terminal::handle_ls(int argc, char* argv[]) {
    const char* path = "/";
    if (argc > 1) {
        path = argv[1];
    }
    fat32->ls(path);
}

void Terminal::handle_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        libc::printf("Usage: mkdir <directory_path>\n");
        return;
    }
    fat32->mkdir(argv[1]);
}

void Terminal::handle_touch(int argc, char* argv[]) {
    if (argc < 2) {
        libc::printf("Usage: touch <file_path>\n");
        return;
    }
    fat32->touch(argv[1]);
}

void Terminal::handle_rm(int argc, char* argv[]) {
    if (argc < 2) {
        libc::printf("Usage: rm <file_path>\n");
        return;
    }
    fat32->rm(argv[1]);
}

void Terminal::handle_rmdir(int argc, char* argv[]) {
    if (argc < 2) {
        libc::printf("Usage: rmdir <directory_path>\n");
        return;
    }
    fat32->rmdir(argv[1]);
}

void Terminal::handle_cat(int argc, char* argv[]) {
    if (argc < 2) {
        libc::printf("Usage: cat <file_path>\n");
        return;
    }
    
    // Ensure path starts with '/' if it's a relative path to root
    char full_path[256];
    if (argv[1][0] != '/' && argv[1][0] != '.') {
        // It's a relative path, prepend '/'
        full_path[0] = '/';
        libc::strncpy(full_path + 1, argv[1], 254);
        full_path[255] = '\0';
    } else {
        // It's already an absolute path or relative path starting with '.'
        libc::strncpy(full_path, argv[1], 255);
        full_path[255] = '\0';
    }
    
    // Debug output to see what path is being passed
    libc::printf("Attempting to open file: '");
    libc::printf(full_path);
    libc::printf("'\n");
    
    int fd = fat32->open(full_path);
    if (fd < 0) {
        libc::printf("Error: Could not open file '");
        libc::printf(full_path);
        libc::printf("'\n");
        return;
    }
    
    uint8_t buffer[512];
    int bytes_read;
    while ((bytes_read = fat32->read(fd, buffer, 511)) > 0) {
        // Print each character individually to avoid buffer overflow issues
        for (int i = 0; i < bytes_read; i++) {
            libc::putchar((char)buffer[i]);
        }
    }
    
    fat32->close(fd);
}

void Terminal::handle_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        libc::printf(argv[i]);
        if (i < argc - 1) {
            libc::printf(" ");
        }
    }
    libc::printf("\n");
}

void Terminal::handle_help() {
    libc::printf("Available commands:\n");
    libc::printf("  ls [path]          - List directory contents\n");
    libc::printf("  mkdir <path>       - Create directory\n");
    libc::printf("  touch <path>       - Create empty file\n");
    libc::printf("  rm <path>          - Remove file\n");
    libc::printf("  rmdir <path>       - Remove directory\n");
    libc::printf("  cat <path>         - Display file contents\n");
    libc::printf("  echo <text>        - Display text\n");
    libc::printf("  clear              - Clear screen\n");
    libc::printf("  help               - Show this help\n");
}

void Terminal::handle_clear() {
    // Clear the screen using our new clear_screen function
    libc::clear_screen();
}

void Terminal::run() {
    // Terminal main loop would go here if we had a proper input system
    // For now, this is just a placeholder
    initialize();
}

} // namespace terminal
} // namespace uqaabOS