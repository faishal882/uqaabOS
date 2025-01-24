#include "../include/drivers/vga.h"

namespace uqaabOS {
namespace driver {

/*
 * Video Graphics Array (VGA) Implementation
 * ---------------------------------------
 * This code implements VGA graphics mode 13h (320x200 with 256 colors).
 * The VGA hardware is controlled through various I/O ports:
 * - Miscellaneous port: General VGA control
 * - CRTC ports: Control timing and scanning of the display
 * - Sequencer ports: Control how data is sent to the display
 * - Graphics Controller ports: Control how graphics data is interpreted
 * - Attribute Controller ports: Control color and display attributes
 *
 * The implementation allows:
 * - Setting up 320x200 graphics mode
 * - Writing pixels directly to video memory
 * - Basic color support
 */

// Constructor initializes all VGA port addresses
VideoGraphicsArray::VideoGraphicsArray()
    // Initialize all VGA port addresses
    : misc_port(0x3c2), crtc_index_port(0x3d4), crtc_data_port(0x3d5),
      sequencer_index_port(0x3c4), sequencer_data_port(0x3c5),
      graphics_controller_index_port(0x3ce),
      graphics_controller_data_port(0x3cf),
      attribute_controller_index_port(0x3c0),
      attribute_controller_read_port(0x3c1),
      attribute_controller_write_port(0x3c0),
      attribute_controller_reset_port(0x3da) {}

VideoGraphicsArray::~VideoGraphicsArray() {}

// Writes configuration data to all VGA registers to set up a specific video
// mode
void VideoGraphicsArray::write_registers(uint8_t *registers) {
  // Writes the first byte from the registers array to the miscellaneous port.
  misc_port.write(*(registers++));

  // Configure sequencer registers (5 registers)
  for (uint8_t i = 0; i < 5; i++) {
    sequencer_index_port.write(i); // Select register index
    sequencer_data_port.write(
        *(registers++)); // Write data to selected register
  }

  // Special handling for CRTC registers to unlock them
  crtc_index_port.write(0x03);
  crtc_data_port.write(crtc_data_port.read() | 0x80);
  crtc_index_port.write(0x11);
  crtc_data_port.write(crtc_data_port.read() & ~0x80);

  // Update register values for vertical retrace end
  registers[0x03] = registers[0x03] | 0x80;
  registers[0x11] = registers[0x11] & ~0x80;

  // Configure CRTC registers (25 registers)
  for (uint8_t i = 0; i < 25; i++) {
    crtc_index_port.write(i);             // Select register index
    crtc_data_port.write(*(registers++)); // Write data to selected register
  }

  // Configure graphics controller registers (9 registers)
  for (uint8_t i = 0; i < 9; i++) {
    graphics_controller_index_port.write(i);
    graphics_controller_data_port.write(*(registers++));
  }

  // Configure attribute controller registers (21 registers)
  for (uint8_t i = 0; i < 21; i++) {
    attribute_controller_reset_port
        .read(); // Reset attribute controller flip-flop
    attribute_controller_index_port.write(i); // Select register index
    attribute_controller_write_port.write(*(registers++)); // Write data
  }

  // Reset attribute controller and enable display
  attribute_controller_reset_port.read();
  attribute_controller_index_port.write(0x20);
}

// Check if the requested video mode is supported
bool VideoGraphicsArray::supports_mode(uint32_t width, uint32_t height,
                                       uint32_t colordepth) {
  // Currently only supports 320x200 with 8-bit color (256 colors)
  return width == 320 && height == 200 && colordepth == 8;
}

// Set the video mode to the requested resolution and color depth
bool VideoGraphicsArray::set_mode(uint32_t width, uint32_t height,
                                  uint32_t colordepth) {
  // Check if mode is supported
  if (!supports_mode(width, height, colordepth))
    return false;

  /* VGA register values for mode 13h (320x200, 256 colors)
   The values for diifernt modes can be found in:
   http://file.osdev.org/mirrors/geezer/osd/graphics/modes.c
  */
  unsigned char g_320x200x256[] = {/* MISC */ 0x63, // Miscellaneous settings
                                   /* SEQ */ 0x03,
                                   0x01,
                                   0x0F,
                                   0x00,
                                   0x0E,      // Sequencer settings
                                   /* CRTC */ // CRT Controller settings
                                   0x5F,
                                   0x4F,
                                   0x50,
                                   0x82,
                                   0x54,
                                   0x80,
                                   0xBF,
                                   0x1F,
                                   0x00,
                                   0x41,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0x9C,
                                   0x0E,
                                   0x8F,
                                   0x28,
                                   0x40,
                                   0x96,
                                   0xB9,
                                   0xA3,
                                   0xFF,
                                   /* GC */ 0x00,
                                   0x00,
                                   0x00,
                                   0x00,
                                   0x00, // Graphics Controller settings
                                   0x40,
                                   0x05,
                                   0x0F,
                                   0xFF,
                                   /* AC */ 0x00,
                                   0x01,
                                   0x02,
                                   0x03,
                                   0x04, // Attribute Controller settings
                                   0x05,
                                   0x06,
                                   0x07,
                                   0x08,
                                   0x09,
                                   0x0A,
                                   0x0B,
                                   0x0C,
                                   0x0D,
                                   0x0E,
                                   0x0F,
                                   0x41,
                                   0x00,
                                   0x0F,
                                   0x00,
                                   0x00};

  // Write the configuration to VGA registers
  write_registers(g_320x200x256);
  return true;
}

// Get the current frame buffer segment address
uint8_t *VideoGraphicsArray::get_frame_buffer_segment() {
  // Read the current memory mapping configuration
  graphics_controller_index_port.write(0x06);
  uint8_t segmentNumber = graphics_controller_data_port.read() & (3 << 2);

  // Return the appropriate memory address based on segment number
  switch (segmentNumber) {
  default:
  case 0 << 2:
    return (uint8_t *)0x00000; // First 64K
  case 1 << 2:
    return (uint8_t *)0xA0000; // VGA memory
  case 2 << 2:
    return (uint8_t *)0xB0000; // Monochrome text mode
  case 3 << 2:
    return (uint8_t *)0xB8000; // Color text mode
  }
}

// Write a pixel to the screen using a color index
void VideoGraphicsArray::put_pixel(uint32_t x, uint32_t y, uint8_t colorIndex) {
  // Calculate pixel address in video memory (320 pixels per row)
  uint8_t *pixelAddress = get_frame_buffer_segment() + 320 * y + x;
  *pixelAddress = colorIndex; // Write the color index to video memory
}

// Convert RGB values to a color index (currently very limited color support)
uint8_t VideoGraphicsArray::get_color_index(uint8_t r, uint8_t g, uint8_t b) {
  // index standard 16-color VGA palette
  if (r == 0xFF && g == 0x00 && b == 0x00)
    return 0x04; // Red color index
  if (r == 0x00 && g == 0xFF && b == 0x00)
    return 0x02; // Green color index
  if (r == 0x00 && g == 0x00 && b == 0xFF)
    return 0x01; // Blue color index
  return 0x00;   // Black color index
}

// Write a pixel to the screen using RGB values
void VideoGraphicsArray::put_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g,
                                   uint8_t b) {
  // Convert RGB to color index and write pixel
  put_pixel(x, y, get_color_index(r, g, b));
}

} // namespace driver
} // namespace uqaabOS