#ifndef __MYOS__GUI__WIDGET_H
#define __MYOS__GUI__WIDGET_H

#include "../drivers/keyboard.h"
#include "./graphicscontext.h"
#include <stdint.h>

namespace uqaabOS {
namespace gui {
class Widget : public driver::KeyboardEventHandler {
protected:
  Widget *parent;
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;

  uint8_t r;
  uint8_t g;
  uint8_t b;

  bool Focussable;

public:
  // constructor
  Widget(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r,
         uint8_t g, uint8_t b);

  // destructor
  ~Widget();

  virtual void GetFocus(Widget *widget);
  virtual void ModelToScreen(int32_t &x, int32_t &y);
  virtual bool ContainsCoordinate(int32_t x, int32_t y);

  virtual void Draw(GraphicsContext *gc);
  virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
  virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
  virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx,
                           int32_t newy);
};

// contains array of widgets and the order decides which gets drawn first
class CompositeWidget : public Widget {
private:
  Widget *children[100];
  int numChildren;
  Widget *focussedChild;

public:
  // constructor
  CompositeWidget(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h,
                  uint8_t r, uint8_t g, uint8_t b);

  // destructor
  ~CompositeWidget();

  virtual void GetFocus(Widget *widget);
  virtual bool AddChild(Widget *child);

  virtual void Draw(GraphicsContext *gc);
  virtual void OnMouseDown(int32_t x, int32_t y, uint8_t button);
  virtual void OnMouseUp(int32_t x, int32_t y, uint8_t button);
  virtual void OnMouseMove(int32_t oldx, int32_t oldy, int32_t newx,
                           int32_t newy);

  virtual void OnKeyDown(char);
  virtual void OnKeyUp(char);
};
} // namespace gui
} // namespace uqaabOS

#endif