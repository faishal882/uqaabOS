// GCC provides these header files automatically
#include <stddef.h>
#include <stdint.h>

// Compiler checks
#if defined(__linux__)
    #error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
    #error "This code must be compiled with an x86-elf compiler"
#endif

// VGA handling class
class VGATerminal {
private:
    static constexpr int VGA_COLS = 80;
    static constexpr int VGA_ROWS = 25;
    
    volatile uint16_t* vga_buffer;
    int term_col;
    int term_row;
    uint8_t term_color;

public:
    // Constructor
    VGATerminal() 
        : vga_buffer(reinterpret_cast<uint16_t*>(0xB8000))
        , term_col(0)
        , term_row(0)
        , term_color(0x0F) // Black background, White foreground
    {
        init();
    }

    // Initialize the terminal
    void init() {
        for (int col = 0; col < VGA_COLS; ++col) {
            for (int row = 0; row < VGA_ROWS; ++row) {
                const size_t index = (VGA_COLS * row) + col;
                vga_buffer[index] = static_cast<uint16_t>(term_color << 8) | ' ';
            }
        }
    }

    // Put a single character on the screen
    void putChar(char c) {
        switch (c) {
            case '\n':
                term_col = 0;
                ++term_row;
                break;
            
            default:
                const size_t index = (VGA_COLS * term_row) + term_col;
                vga_buffer[index] = static_cast<uint16_t>(term_color << 8) | c;
                ++term_col;
                break;
        }

        // Handle column overflow
        if (term_col >= VGA_COLS) {
            term_col = 0;
            ++term_row;
        }

        // Handle row overflow
        if (term_row >= VGA_ROWS) {
            term_col = 0;
            term_row = 0;
        }
    }

    // Print a string
    void print(const char* str) {
        for (size_t i = 0; str[i] != '\0'; ++i) {
            putChar(str[i]);
        }
    }
};

// Kernel entry point
extern "C" void kernel_main() {
    // Create our terminal instance
    VGATerminal terminal;

    // Display some messages
    terminal.print("Hello, World!\n");
    terminal.print("Welcome to the C++ kernel.\n");
}
