#ifndef XORG_WINDOW_INFO_H
#define XORG_WINDOW_INFO_H

#include <godot_cpp/classes/resource.hpp>

namespace godot {

class XorgWindowInfo : public Resource {
  GDCLASS(XorgWindowInfo, Resource);

private:
  String wm_name;
  String wm_class;
  Rect2i wm_rect;
protected:
  static void _bind_methods();
public:
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
