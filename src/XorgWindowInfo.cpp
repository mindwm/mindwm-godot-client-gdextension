#include "XorgWindowInfo.h"

using namespace godot;

void XorgWindowInfo::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_win_id"), &XorgWindowInfo::get_win_id);
  ClassDB::bind_method(D_METHOD("get_parent_id"), &XorgWindowInfo::get_parent_id);
  ClassDB::bind_method(D_METHOD("get_name"), &XorgWindowInfo::get_wm_name);
  ClassDB::bind_method(D_METHOD("get_class"), &XorgWindowInfo::get_wm_class);
  ClassDB::bind_method(D_METHOD("get_rect"), &XorgWindowInfo::get_wm_rect);
}

xcb_window_t XorgWindowInfo::get_win_id(){
  return win_id;
}

void XorgWindowInfo::set_win_id(xcb_window_t wid){
  win_id = wid;
}

xcb_window_t XorgWindowInfo::get_parent_id(){
  return parent_id;
}

void XorgWindowInfo::set_parent_id(xcb_window_t wid){
  parent_id = wid;
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
