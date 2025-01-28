#include "../include/gui/desktop.h"

namespace uqaabOS {
namespace gui {
//>class Desktop

//>protected:

uint32_t MouseX;
uint32_t MouseY;

//<>public:

// constructor
Desktop::Desktop(int32_t w, int32_t h, uint8_t r, uint8_t g, uint8_t b)
    : CompositeWidget(0, 0, 0, w, h, r, g, b), MouseEventHandler() {
  // initialize mouse in middle of screen.
  // 0x, 0y is top left
  // increaing x goes right
  // increasing y goes down
  MouseX = w / 2;
  MouseY = h / 2;
}

// destructor
Desktop::~Desktop() {}

void Desktop::Draw(GraphicsContext *gc) {

  // draw desktop
  CompositeWidget::Draw(gc);

  // draw mouse cursor
  // cursor is a white cross
  for (int i = 0; i < 4; i++) {
    gc->put_pixel(MouseX - i, MouseY, 0xFF, 0xFF, 0xFF);
    gc->put_pixel(MouseX + i, MouseY, 0xFF, 0xFF, 0xFF);
    gc->put_pixel(MouseX, MouseY - i, 0xFF, 0xFF, 0xFF);
    gc->put_pixel(MouseX, MouseY + i, 0xFF, 0xFF, 0xFF);
  }
}

void Desktop::OnMouseDown(uint8_t button) {
  CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseUp(uint8_t button) {
  CompositeWidget::OnMouseDown(MouseX, MouseY, button);
}

void Desktop::OnMouseMove(int x, int y) {

  // slow down mouse movement for desired sensativity
  x /= 4;
  y /= 4;

  //>make sure mouse stays inside desktop screen

  // calculate new x coord
  int32_t newMouseX = MouseX + x;
  if (newMouseX < 0)
    newMouseX = 0;
  if (newMouseX >= w)
    newMouseX = w - 1;

  // calculate new y coord
  int32_t newMouseY = MouseY + y;
  if (newMouseY < 0)
    newMouseY = 0;
  if (newMouseY >= h)
    newMouseY = h - 1;
  //<

  // pass on movment to CompositeWidget
  CompositeWidget::OnMouseMove(MouseX, MouseY, newMouseX, newMouseY);

  // store new values back in coordinates
  MouseX = newMouseX;
  MouseY = newMouseY;
}
//<
//<
} // namespace gui
} // namespace uqaabOS