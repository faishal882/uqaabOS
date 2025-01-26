#ifndef __DESKTOP_H
#define __DESKTOP_H

#include "../drivers/mouse.h"
#include "./widget.h"

namespace uqaabOS {
namespace gui {

class Desktop : public CompositeWidget, public driver::MouseEventHandler {
protected:
  uint32_t mouse_x;
  uint32_t mouse_y;

public:
  Desktop(int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b);
  ~Desktop();

  void Draw(GraphicsContext *gc);

  void on_mouse_down(uint8_t button);
  void on_mouse_up(uint8_t button);
  void on_mouse_move(int x, int y);
};

} // namespace gui
} // namespace uqaabOS
#endif