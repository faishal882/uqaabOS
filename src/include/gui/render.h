#ifndef __MYOS__GUI__RENDER_H
#define __MYOS__GUI__RENDER_H

#include "./../drivers/vga.h"
#include "./graphicscontext.h"
#include <stdint.h>

namespace uqaabOS {
namespace gui {

class Pixel {
public:
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

class Render : public driver::VideoGraphicsArray {
private:
  Pixel pixels[320][200];

public:
  Render(int32_t w, int32_t h);

  ~Render();

  void display(GraphicsContext *gc);

  void PutPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b);
};
} // namespace gui
} // namespace uqaabOS

#endif