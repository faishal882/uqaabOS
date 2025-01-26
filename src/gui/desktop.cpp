#include "../include/gui/desktop.h"

namespace uqaabOS {
namespace gui {

Desktop::Desktop(int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b)
    : CompositeWidget(0, 0, 0, w, h, r, g, b), MouseEventHandler() {
  mouse_x = w / 2;
  mouse_y = h / 2;
}
Desktop::~Desktop() {}
void Desktop::Draw(GraphicsContext *gc) {
  CompositeWidget::draw(gc);

  for (int i = 0; i < 4; i++) {
    gc->put_pixel(mouse_x - i, mouse_y, 0xFF, 0xFF, 0xFF);
    gc->put_pixel(mouse_x + i, mouse_y, 0xFF, 0xFF, 0xFF);
    gc->put_pixel(mouse_x, mouse_y - i, 0xFF, 0xFF, 0xFF);
    gc->put_pixel(mouse_x, mouse_y + i, 0xFF, 0xFF, 0xFF);
  }
}

void Desktop::on_mouse_down(uint8_t button) {
  CompositeWidget::on_mouse_down(mouse_x, mouse_y, button);
}
void Desktop::on_mouse_up(uint8_t button) {
  CompositeWidget::on_mouse_up(mouse_x, mouse_y, button);
}
void Desktop::on_mouse_move(int x, int y) {
  x /= 4;
  y /= 4;

  int32_t newmouse_x = mouse_x + x;
  if (newmouse_x < 0)
    newmouse_x = 0;
  if (newmouse_x >= w)
    newmouse_x = w - 1;

  int32_t newmouse_y = mouse_y + y;
  if (newmouse_y < 0)
    newmouse_y = 0;
  if (newmouse_y >= h)
    newmouse_y = h - 1;

  CompositeWidget::on_mouse_move(mouse_x, mouse_y, newmouse_x, newmouse_y);

  mouse_x = newmouse_x;
  mouse_y = newmouse_y;
}

} // namespace gui
} // namespace uqaabOS