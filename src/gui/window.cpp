#include "../include/gui/window.h"

namespace uqaabOS {
namespace gui {

Window::Window(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h,
               uint8_t r, uint8_t g, uint8_t b)
    : CompositeWidget(parent, x, y, w, h, r, g, b) {
  Dragging = false;
}
Window::~Window() {}
void Window::on_mouse_down(int32_t x, int32_t y, uint8_t button) {
  Dragging = button == 1;
  CompositeWidget::on_mouse_down(x, y, button);
}
void Window::on_mouse_up(int32_t x, int32_t y, uint8_t button) {
  Dragging = false;
  CompositeWidget::on_mouse_up(x, y, button);
}
void Window::on_mouse_move(int32_t oldx, int32_t oldy, int32_t newx,
                           int32_t newy) {
  if (Dragging) {
    this->x += newx - oldx;
    this->y += newy - oldy;
  }
  CompositeWidget::on_mouse_move(oldx, oldy, newx, newy);
}

} // namespace gui
} // namespace uqaabOS