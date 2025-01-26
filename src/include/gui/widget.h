#ifndef __WIDGET_H
#define __WIDGET_H

#include "../drivers/keyboard.h"
#include "./graphicscontext.h"
#include <cstdint>
#include <stdint.h>

namespace uqaabOS {
namespace gui {

/**
 *  Base class for all GUI widgets.
 *
 * This class represents a basic GUI widget with properties such as position,
 * size, color, and focusability. It provides virtual methods for handling
 * drawing, mouse events, and keyboard events.
 */
class Widget : public driver::KeyboardEventHandler {
protected:
  Widget *parent; // Pointer to the parent widget.
  int32_t x;      // X position of the widget.
  int32_t y;      // Y position of the widget.
  int32_t w;      // Width of the widget.
  int32_t h;      // Height of the widget.

  uint8_t r;       // Red component of the widget's color.
  uint8_t g;       // Green component of the widget's color.
  uint8_t b;       // Blue component of the widget's color.
  bool focussable; // Indicates if the widget can receive focus.

public:
  Widget(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t r,
         uint8_t g, uint8_t b);

  ~Widget();

  /**
   * Method to give focus to a widget.
   */
  virtual void get_focus(Widget *widget);

  /**
   *  Converts model coordinates to screen coordinates.
   */
  virtual void model_to_screen(int32_t &x, int32_t &y);
  virtual bool contains_coordiante(int32_t x, int32_t y);

  /**
   *  Draws the widget using the provided graphics context.
   */
  virtual void draw(GraphicsContext *gc);

  /**
   *  Handles mouse down events.
   */
  virtual void on_mouse_down(int32_t x, int32_t y, uint8_t button);

  /**
   *  Handles mouse up events.
   */
  virtual void on_mouse_up(int32_t x, int32_t y, uint8_t button);

  /**
   *  Handles mouse move events.
   */
  virtual void on_mouse_move(int32_t oldx, int32_t oldy, int32_t newx,
                             int32_t newy);
};

/**
 * CompositeWidget
 *  A widget that can contain child widgets.
 *
 * This class represents a composite widget that can contain multiple child
 * widgets. It provides methods for managing focus and handling events for
 * child widgets.
 */
class CompositeWidget : public Widget {
private:
  Widget *children[100];  // Array of pointers to child widgets.
  int num_children;       // Number of child widgets.
  Widget *focussed_child; // Pointer to the currently focused child widget.

public:
  CompositeWidget(Widget *parent, int32_t x, int32_t y, int32_t w, int32_t h,
                  uint8_t r, uint8_t g, uint8_t b);
  ~CompositeWidget();

  /**
   *  Method to give focus to a widget.
   */
  virtual void get_focus(Widget *widget);
  virtual bool add_child(Widget *child);

  /**
   *  Draws the composite widget and its children using the provided
   * graphics context.
   */
  virtual void draw(GraphicsContext *gc);

  /**
   *  Handles mouse down events.
   */
  virtual void on_mouse_down(int32_t x, int32_t y, uint8_t button);

  /**
   *  Handles mouse up events.
   */
  virtual void on_mouse_up(int32_t x, int32_t y, uint8_t button);

  /**
   *  Handles mouse move events.
   */
  virtual void on_mouse_move(int32_t oldx, int32_t oldy, int32_t newx,
                             int32_t newy);

  /**
   *  Handles key down events.
   */
  virtual void on_key_down(char);

  /**
   *  Handles key up events.
   */
  virtual void on_key_up(char);
};

} // namespace gui
} // namespace uqaabOS
#endif