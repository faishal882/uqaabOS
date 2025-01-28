#include "../include/gui/widget.h"
#include <cstdint>

namespace uqaabOS {
namespace gui {
//>class Widget

// constructor
Widget::Widget(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h,
               uint8_t r, uint8_t g, uint8_t b)
    : KeyboardEventHandler() {
  this->parent = parent;

  // position
  this->x = x;
  this->y = y;

  // size
  this->w = w;
  this->h = h;

  // rgb color attributes
  this->r = r;
  this->g = g;
  this->b = b;

  this->Focussable = true;
}

// destructor
Widget::~Widget() {}

void Widget::GetFocus(Widget *widget) {

  // pass focus to parent
  // all widgets will pass focus to parents untill a parent contains the
  // functionality to deal with it for example if you click a text box, tell the
  // window containing that text box that it is in focus
  if (parent != 0) {
    parent->GetFocus(widget);
  }
}

// to get absolute coordinates
void Widget::ModelToScreen(int32_t &x, int32_t &y) {

  // get offset from parent
  if (parent != 0) {
    parent->ModelToScreen(x, y);
  }

  // add offset to this widget
  x += this->x;
  y += this->y;
}

void Widget::Draw(GraphicsContext *gc) {

  // get absolute coordinates so we know where to draw
  int32_t X = 0;
  int32_t Y = 0;
  ModelToScreen(X, Y);

  gc->fill_rectangle(X, Y, w, h, r, g, b);
}

void Widget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
  if (Focussable) {
    GetFocus(this);
  }
}

// check if x and y are larger than x and y coordinates of the widget, and
// smaller than the coordinates + width or height respectivly
bool Widget::ContainsCoordinate(int32_t x, int32_t y) {
  return this->x <= x && x < this->x + this->w && this->y <= y &&
         y < this->y + this->h;
}

void Widget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {}

void Widget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx,
                         int32_t newy) {}

//<>class CompositeWidget

// constructor
CompositeWidget::CompositeWidget(Widget *parent, int32_t x, int32_t y,
                                 int32_t w, int32_t h, uint8_t r, uint8_t g,
                                 uint8_t b)
    : Widget(parent, x, y, w, h, r, g, b) {
  focussedChild = 0;
  numChildren = 0;
}

// destructor
CompositeWidget::~CompositeWidget() {}

void CompositeWidget::GetFocus(Widget *widget) {
  this->focussedChild = widget;
  if (parent != 0) {
    parent->GetFocus(this);
  }
}

bool CompositeWidget::AddChild(Widget *child) {
  if (numChildren >= 100) {
    return false;
  }
  children[numChildren++] = child;
  return true;
}

void CompositeWidget::Draw(GraphicsContext *gc) {
  // draw own background
  Widget::Draw(gc);

  // draw all children
  for (int i = numChildren - 1; i >= 0; --i) {
    children[i]->Draw(gc);
  }
}

// iterate through children and pass event to the child that contains the
// coorinate pass the click to the one that has been clicked
void CompositeWidget::OnMouseDown(int32_t x, int32_t y, uint8_t button) {
  for (int i = 0; i < numChildren; ++i) {
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
      children[i]->OnMouseDown(x - this->x, y - this->y, button);

      // only the top window is clicked on
      break;
    }
  }
}

// iterate through children and pass event to the child that contains the
// coorinate pass the click to the one that has been clicked
void CompositeWidget::OnMouseUp(int32_t x, int32_t y, uint8_t button) {
  for (int i = 0; i < numChildren; ++i) {
    if (children[i]->ContainsCoordinate(x - this->x, y - this->y)) {
      children[i]->OnMouseUp(x - this->x, y - this->y, button);

      // only the top window is clicked on
      break;
    }
  }
}

void CompositeWidget::OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx,
                                  int32_t newy) {

  int firstChild = -1;

  // for object that contains old coordinate
  for (int i = 0; i < numChildren; ++i) {
    if (children[i]->ContainsCoordinate(oldx - this->x, oldy - this->y)) {

      // subtracting thisx/y turns the coordinates into relative coordinates
      children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x,
                               newy - this->y);

      firstChild = i;

      // only the top window is clicked on
      break;
    }
  }

  // for object that contains new coordinate
  for (int i = 0; i < numChildren; ++i) {
    if (children[i]->ContainsCoordinate(newx - this->x, newy - this->y)) {

      if (firstChild != i) {
        // subtracting thisx/y turns the coordinates into relative coordinates
        children[i]->OnMouseMove(oldx - this->x, oldy - this->y, newx - this->x,
                                 newy - this->y);
      }

      // only the top window is clicked on
      break;
    }
  }
}

void CompositeWidget::OnKeyDown(char str) {
  if (focussedChild != 0) {
    focussedChild->on_key_down(str);
  }
}

void CompositeWidget::OnKeyUp(char str) {
  if (focussedChild != 0) {
    focussedChild->on_key_up(str);
  }
}
//<
} // namespace gui
} // namespace uqaabOS