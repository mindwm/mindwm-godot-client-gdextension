#ifndef XORG_H
#define XORG_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <xcb/xcb.h>
#include "XorgWindowInfo.h"

namespace godot {

class Xorg : public Node {
	GDCLASS(Xorg, Node)

private:
  Ref<Thread> eventsWatcher;
  xcb_connection_t* conn;
  Vector<Ref<XorgWindowInfo>> windows;
	double time_passed;

  xcb_atom_t get_atom(const char *atom_name);
  xcb_get_property_reply_t* get_win_property(xcb_window_t win, xcb_atom_t atom);
  String get_win_text_property(xcb_window_t win, xcb_atom_t atom);
  void list_xorg_windows();
  void watchEvents();

protected:
	static void _bind_methods();
  void _notification(int p_what);

public:
	Xorg();
	~Xorg();

  Ref<XorgWindowInfo> get_window(int p_index);
  TypedArray<XorgWindowInfo> list_windows();
  void refresh_xorg_windows();

  void init();

  void _ready() override;
	void _process(double delta) override;
};

}

#endif
