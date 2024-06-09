#ifndef XORG_WINDOW_INFO_H
#define XORG_WINDOW_INFO_H

#include <godot_cpp/classes/resource.hpp>
#include <xcb/xcb.h>

namespace godot {

class XorgWindowInfo : public Resource {
  GDCLASS(XorgWindowInfo, Resource);

private:
  xcb_window_t win_id;
  xcb_window_t parent_id;
  String wm_name;
  String wm_class;
  Rect2i wm_rect;
protected:
  static void _bind_methods();
public:
  xcb_window_t get_win_id();
  void set_win_id(xcb_window_t win_id);

  xcb_window_t get_parent_id();
  void set_parent_id(xcb_window_t parent_id);

  String get_wm_name();
  void set_wm_name(String);

  String get_wm_class();
  void set_wm_class(String);

  Rect2i get_wm_rect();
  void set_wm_rect(Rect2i);

  XorgWindowInfo();
  ~XorgWindowInfo();
};
}

#endif
