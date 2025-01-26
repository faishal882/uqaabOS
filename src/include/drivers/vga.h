#ifndef __VGA_H
#define __VGA_H

#include "../port.h"
#include "./driver.h"
#include <stdint.h>

namespace uqaabOS {
namespace driver {

/*
 * VideoGraphicsArray Class
 * -----------------------
 * This class provides an interface to the VGA hardware, specifically targeting
 * mode 13h (320x200 with 256 colors). The VGA hardware is controlled through
 * various I/O ports that are grouped into several functional units:
 *
 * 1. Miscellaneous Register:
 *    - Controls basic VGA configuration
 *    - Handles sync signals and clock selection
 *
 * 2. CRT Controller (CRTC):
 *    - Controls timing signals
 *    - Manages screen refresh
 *    - Handles cursor display
 *
 * 3. Sequencer:
 *    - Controls how data is sent to display
 *    - Manages character clocking
 *    - Handles memory plane operations
 *
 * 4. Graphics Controller:
 *    - Controls how graphics data is interpreted
 *    - Manages read/write modes
 *    - Handles bit masking and rotation
 *
 * 5. Attribute Controller:
 *    - Controls color and attribute settings
 *    - Manages palette registers
 *    - Handles border color
 */
/*
 *    To implemt higher resolution(eg: 1366x768 VGA mode) is to be implemnted
 *    using VESA (Video Electronics Standards Association) BIOS Extensions (VBE)
 *    standard. VBE is a graphics standard that supports higher resolutions and
 *    color depths compared to traditional VGA.
 */

class VideoGraphicsArray {
protected:
  // Port objects for VGA register access
  include::Port8Bit misc_port;       // 0x3c2 - Miscellaneous Output Register
  include::Port8Bit crtc_index_port; // 0x3d4 - CRTC Index Register
  include::Port8Bit crtc_data_port;  // 0x3d5 - CRTC Data Register
  include::Port8Bit sequencer_index_port; // 0x3c4 - Sequencer Index Register
  include::Port8Bit sequencer_data_port;  // 0x3c5 - Sequencer Data Register
  include::Port8Bit
      graphics_controller_index_port; // 0x3ce - Graphics Controller Index
  include::Port8Bit
      graphics_controller_data_port; // 0x3cf - Graphics Controller Data
  include::Port8Bit
      attribute_controller_index_port; // 0x3c0 - Attribute Controller Index
  include::Port8Bit
      attribute_controller_read_port; // 0x3c1 - Attribute Controller Read
  include::Port8Bit
      attribute_controller_write_port; // 0x3c0 - Attribute Controller Write
  include::Port8Bit
      attribute_controller_reset_port; // 0x3da - Attribute Controller Reset

  /*
   * Protected Methods
   * ----------------
   * These methods handle the low-level VGA hardware interaction
   */

  // Writes configuration data to all VGA registers
  // @param registers: Pointer to array of register values
  void write_registers(uint8_t *registers);

  // Gets the current frame buffer memory segment
  // @return: Pointer to the start of frame buffer memory
  uint8_t *get_frame_buffer_segment();

  // Converts RGB values to a color index in the palette
  // @param r, g, b: Color components (0-255)
  // @return: Color index in the VGA palette
  virtual uint8_t get_color_index(uint8_t r, uint8_t g, uint8_t b);

public:
  /*
   * Public Interface
   * ---------------
   * These methods provide the main functionality for graphics operations
   */

  // Constructor - Initializes all VGA ports
  VideoGraphicsArray();

  // Destructor - Clean up resources
  ~VideoGraphicsArray();

  // Checks if a specific video mode is supported
  // @param width: Screen width in pixels
  // @param height: Screen height in pixels
  // @param colordepth: Bits per pixel
  // @return: true if mode is supported
  virtual bool supports_mode(uint32_t width, uint32_t height,
                             uint32_t colordepth);

  // Sets the video mode to specified parameters
  // @param width: Screen width in pixels
  // @param height: Screen height in pixels
  // @param colordepth: Bits per pixel
  // @return: true if mode was set successfully
  virtual bool set_mode(uint32_t width, uint32_t height, uint32_t colordepth);

  // Draws a pixel using RGB values
  // @param x, y: Pixel coordinates
  // @param r, g, b: Color components (0-255)
  virtual void put_pixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b);

  // Draws a pixel using a palette index
  // @param x, y: Pixel coordinates
  // @param colorIndex: Index into the color palette
  virtual void put_pixel(int32_t x, int32_t y, uint8_t colorIndex);

  /*
   * Draws a filled rectangle using RGB values
   * @param x, y: Top-left corner coordinates of the rectangle
   * @param w, h: Width and height of the rectangle
   * @param r, g, b: Color components (0-255)
   */
  virtual void fill_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                              uint8_t r, uint8_t g, uint8_t b);
};

} // namespace driver
} // namespace uqaabOS

#endif

/*
 * Usage Notes
 * ----------
 * 1. Initialize the VGA driver:
 *    VideoGraphicsArray vga;
 *
 * 2. Set video mode:
 *    vga.set_mode(320, 200, 8);
 *
 * 3. Draw pixels:
 *    vga.put_pixel(x, y, r, g, b);
 *    - or -
 *    vga.put_pixel(x, y, colorIndex);
 *
 * Limitations
 * ----------
 * - Currently only supports 320x200 with 256 colors (Mode 13h)
 * - Limited color palette support
 * - No hardware acceleration features
 * - No support for text modes
 */
