#include "XorgWindowInfo.h"

using namespace godot;

void XorgWindowInfo::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_name"), &XorgWindowInfo::get_wm_name);
  ClassDB::bind_method(D_METHOD("get_class"), &XorgWindowInfo::get_wm_class);
  ClassDB::bind_method(D_METHOD("get_rect"), &XorgWindowInfo::get_wm_rect);
}

String XorgWindowInfo::get_wm_name() {
  return wm_name;
}

void XorgWindowInfo::set_wm_name(String n) {
  wm_name = n;
}

String XorgWindowInfo::get_wm_class() {
  return wm_class;
}

void XorgWindowInfo::set_wm_class(String c) {
  wm_class = c;
}


Rect2i XorgWindowInfo::get_wm_rect() {
  return wm_rect;
}

void XorgWindowInfo::set_wm_rect(Rect2i r) {
  wm_rect = r;
}



XorgWindowInfo::XorgWindowInfo() {
}

XorgWindowInfo::~XorgWindowInfo() {
}
