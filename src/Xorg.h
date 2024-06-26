#ifndef XORG_H
#define XORG_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <xcb/xcb.h>
#include "XorgWindowInfo.h"
#include "XorgWindowTexture.h"

namespace godot {

class Xorg : public Node {
	GDCLASS(Xorg, Node)

private:
  void *disp = NULL;
  Ref<Thread> eventsWatcher;
  Ref<Thread> clipboardWatcher;
  bool clipboardWatcherTerminated;
  bool eventsWatcherTerminated;
  xcb_connection_t* conn;
  Vector<Ref<XorgWindowInfo>> windows;
  Vector<Ref<XorgWindowTexture>> window_textures;
	double time_passed;

  void send_xorg_dummy_event();
  xcb_atom_t get_atom(const char *atom_name);
  xcb_window_t get_window_parent(xcb_window_t win);
  xcb_get_property_reply_t* get_win_property(xcb_window_t win, xcb_atom_t atom);
  xcb_atom_t intern_atom(const char *str);
  bool xcomp_check_ewmh(xcb_window_t root);
  String get_win_text_property(xcb_window_t win, xcb_atom_t atom);
  void add_window(xcb_window_t win, xcb_window_t parent);
  void remove_window(Ref<XorgWindowInfo> elem);
  void configure_window(Ref<XorgWindowInfo> w, Rect2i rect);
  void list_xorg_windows();
  void watchEvents();
  void watchClipboard();
  void capture_window(int p_window_index);

protected:
	static void _bind_methods();
  void _notification(int p_what);

public:
	Xorg();
	~Xorg();

  void set_net_wm_desktop(int p_index, uint64_t val);
  Ref<XorgWindowInfo> get_wm_window(int p_index);
  Ref<XorgWindowTexture> get_wm_window_texture(int p_index);
  Ref<XorgWindowInfo> find_window_by_id(xcb_window_t win_id);
  Ref<XorgWindowInfo> find_window_by_parent_id(xcb_window_t parent_id);
  TypedArray<XorgWindowInfo> list_windows();
  void refresh_xorg_windows();

  void init();

  void _ready() override;
	void _process(double delta) override;
};

}

#endif
