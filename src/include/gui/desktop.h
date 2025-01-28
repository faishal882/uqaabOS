#ifndef __MYOS__GUI__DESKTOP_H
#define __MYOS__GUI__DESKTOP_H

#include "../drivers/mouse.h"
#include "./widget.h"

namespace uqaabOS {
namespace gui {
class Desktop : public CompositeWidget, public driver::MouseEventHandler {
protected:
  uint32_t MouseX;
  uint32_t MouseY;

public:
  Desktop(int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b);

  ~Desktop();

  void Draw(GraphicsContext *gc);

  void OnMouseDown(uint8_t button);
  void OnMouseUp(uint8_t button);
  void OnMouseMove(int x, int y);
};
} // namespace gui
} // namespace uqaabOS

#endif