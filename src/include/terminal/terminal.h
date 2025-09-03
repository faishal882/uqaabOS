#ifndef __TERMINAL_H
#define __TERMINAL_H

#include "../filesystem/fat32.h"
#include "../libc/stdio.h"
#include "../libc/string.h"

namespace uqaabOS {
namespace terminal {

#define TERMINAL_BUFFER_SIZE 256
#define MAX_ARGS 32

class Terminal {
private:
    filesystem::FAT32* fat32;
    char input_buffer[TERMINAL_BUFFER_SIZE];
    int buffer_position;
    
    // Command handlers
    void handle_ls(int argc, char* argv[]);
    void handle_mkdir(int argc, char* argv[]);
    void handle_touch(int argc, char* argv[]);
    void handle_rm(int argc, char* argv[]);
    void handle_rmdir(int argc, char* argv[]);
    void handle_cat(int argc, char* argv[]);
    void handle_write(int argc, char* argv[]);
    void handle_echo(int argc, char* argv[]);
    void handle_help();
    void handle_clear();
    
    // Utility functions
    int parse_command(char* command, char* argv[]);
    void print_prompt();
    void execute_command(char* command);
    
public:
    Terminal(filesystem::FAT32* fat32_instance);
    void initialize();
    void run();
    void handle_key_press(char c);
};

} // namespace terminal
} // namespace uqaabOS

#endif // __TERMINAL_H