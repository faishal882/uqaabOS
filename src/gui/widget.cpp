#include "../include/gui/widget.h"
#include <cstdint>

namespace uqaabOS {
namespace gui {
Widget::Widget(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h,
               uint8_t r, uint8_t g, uint8_t b)
    : driver::KeyboardEventHandler() {
  this->parent = parent;
  this->x = x;
  this->y = y;
  this->w = w;
  this->h = h;
  this->r = r;
  this->g = g;
  this->b = b;
  this->focussable = true;
}
Widget::~Widget() {}

void Widget::get_focus(Widget *widget) {
  if (parent != 0)
    parent->get_focus(widget);
}
void Widget::model_to_screen(int32_t &x, int32_t &y) {
  if (parent != 0)
    parent->model_to_screen(x, y);
  x += this->x;
  y += this->y;
}

void Widget::draw(GraphicsContext *gc) {
  int32_t X = 0;
  int32_t Y = 0;
  model_to_screen(X, Y);
  gc->fill_rectangle(X, Y, w, h, r, g, b);
}
void Widget::on_mouse_down(int32_t x, int32_t y, uint8_t button) {
  if (focussable)
    get_focus(this);
}

bool Widget::contains_coordiante(int32_t x, int32_t y) {
  return this->x <= x && x < this->x + this->w && this->y <= y &&
         y < this->y + this->h;
}

void Widget::on_mouse_up(int32_t x, int32_t y, uint8_t button) {}
void Widget::on_mouse_move(int32_t oldx, int32_t oldy, int32_t newx,
                           int32_t newy) {}

CompositeWidget::CompositeWidget(Widget *parent, int32_t x, int32_t y,
                                 int32_t w, int32_t h, uint8_t r, uint8_t g,
                                 uint8_t b)
    : Widget(parent, x, y, w, h, r, g, b) {
  focussed_child = 0;
  num_children = 0;
}

CompositeWidget::~CompositeWidget() {}

void CompositeWidget::get_focus(Widget *widget) {
  this->focussed_child = widget;
  if (parent != 0)
    parent->get_focus(this);
}

bool CompositeWidget::add_child(Widget *child) {
  if (num_children >= 100)
    return false;
  children[num_children++] = child;
  return true;
}

void CompositeWidget::draw(GraphicsContext *gc) {
  Widget::draw(gc);
  for (int i = num_children - 1; i >= 0; --i)
    children[i]->draw(gc);
}
void CompositeWidget::on_mouse_down(int32_t x, int32_t y, uint8_t button) {
  for (int i = 0; i < num_children; ++i)
    if (children[i]->contains_coordiante(x - this->x, y - this->y)) {
      children[i]->on_mouse_down(x - this->x, y - this->y, button);
      break;
    }
}
void CompositeWidget::on_mouse_up(int32_t x, int32_t y, uint8_t button) {
  for (int i = 0; i < num_children; ++i)
    if (children[i]->contains_coordiante(x - this->x, y - this->y)) {
      children[i]->on_mouse_up(x - this->x, y - this->y, button);
      break;
    }
}
void CompositeWidget::on_mouse_move(int32_t oldx, int32_t oldy, int32_t newx,
                                    int32_t newy) {
  int firstchild = -1;
  for (int i = 0; i < num_children; ++i)
    if (children[i]->contains_coordiante(oldx - this->x, oldy - this->y)) {
      children[i]->on_mouse_move(oldx - this->x, oldy - this->y, newx - this->x,
                                 newy - this->y);
      firstchild = i;
      break;
    }
  for (int i = 0; i < num_children; ++i)
    if (children[i]->contains_coordiante(newx - this->x, newy - this->y)) {
      if (firstchild != i)
        children[i]->on_mouse_move(oldx - this->x, oldy - this->y,
                                   newx - this->x, newy - this->y);
      break;
    }
}
void CompositeWidget::on_key_down(char str) {
  if (focussed_child != 0)
    focussed_child->on_key_down(str);
}
void CompositeWidget::on_key_up(char str) {
  if (focussed_child != 0)
    focussed_child->on_key_up(str);
}

} // namespace gui
} // namespace uqaabOS