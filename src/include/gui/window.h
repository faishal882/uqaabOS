#ifndef __WINDOW_H
#define __WINDOW_H

#include "../drivers/mouse.h"
#include "./widget.h"

namespace uqaabOS {
namespace gui {

class Window : public CompositeWidget {
protected:
  bool Dragging;

public:
  Window(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r,
         uint8_t g, uint8_t b);
  ~Window();
  void on_mouse_down(int32_t x, int32_t y, uint8_t button);
  void on_mouse_up(int32_t x, int32_t y, uint8_t button);
  void on_mouse_move(int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);
};
} // namespace gui
} // namespace uqaabOS
#endif