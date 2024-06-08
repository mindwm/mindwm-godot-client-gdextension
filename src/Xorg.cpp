#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <xcb/xcb.h>
#include "Xorg.h"

using namespace godot;

void Xorg::_bind_methods() {
  ClassDB::bind_method(D_METHOD("init"), &Xorg::init);
  ClassDB::bind_method(D_METHOD("list_windows"), &Xorg::list_windows);
  ClassDB::bind_method(D_METHOD("refresh_xorg_windows"), &Xorg::refresh_xorg_windows);
}

Xorg::Xorg() {
  // Initialize any variables here.
  time_passed = 0.0;
}

void Xorg::init(){
  conn = xcb_connect(NULL, NULL);
  ERR_FAIL_NULL(conn);
  UtilityFunctions::print("[mindwm]: connected to X11 server");
}

Xorg::~Xorg() {
  // Add your cleanup here.
}

void Xorg::_ready() {
  UtilityFunctions::print("[mindwm]: Xorg module initialized");
}

Ref<XorgWindowInfo> Xorg::get_window(int p_index) {
  ERR_FAIL_INDEX_V(p_index, windows.size(), nullptr);

  return windows[p_index];
}

TypedArray<XorgWindowInfo> Xorg::list_windows() {
  TypedArray<XorgWindowInfo> ws;
  ws.resize(windows.size());

  for (int i = 0; i < windows.size(); i++) {
    ws[i] = get_window(i);
  }

  return ws;
}

void Xorg::_notification(int p_what) {
  // Don't run if we're in the editor
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  switch (p_what) {
  case NOTIFICATION_READY: {
    eventsWatcher.instantiate();
    eventsWatcher->start(callable_mp(this, &Xorg::watchEvents), Thread::PRIORITY_NORMAL);
  } break;
  case NOTIFICATION_WM_CLOSE_REQUEST: {
    if (eventsWatcher.is_valid()) {
      eventsWatcher->wait_to_finish();
    }

    eventsWatcher.unref();
  };
  }
}

void Xorg::_process(double delta) {
  time_passed += delta;
}

void Xorg::watchEvents() {
  xcb_connection_t *conn;
  conn = xcb_connect(NULL, NULL);

  xcb_screen_t *screen;
  xcb_window_t root_window;
  xcb_generic_event_t *event;
  xcb_void_cookie_t cookie;

  screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
  root_window = screen->root;

  cookie =
    xcb_change_window_attributes_checked(
        conn, root_window, XCB_CW_EVENT_MASK,
        (const uint32_t[]){XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY});
  xcb_flush(conn);

  ERR_FAIL_COND(xcb_request_check(conn, cookie));

  while ((event = xcb_wait_for_event(conn))) {
    switch (event->response_type & ~0x80) {
      case XCB_CREATE_NOTIFY:
        xcb_create_notify_event_t *create_event = (xcb_create_notify_event_t *)event;
        if (create_event->width > 1 || create_event->height > 1) {
          UtilityFunctions::print(vformat("new window event: 0x%08x", create_event->window));
          UtilityFunctions::print(vformat("\tWxH (%dx%d)", create_event->width, create_event->height));
        }
        else {
          // maybe some utility windows
        }
        break;
    }
    std::free(event);
  }
  xcb_disconnect(conn);
}

// utility_functions
xcb_atom_t Xorg::get_atom(const char *atom_name){
  xcb_intern_atom_cookie_t atom_c = xcb_intern_atom(conn, 1, strlen(atom_name), atom_name);
  xcb_intern_atom_reply_t *atom = xcb_intern_atom_reply(conn, atom_c, NULL);
  xcb_atom_t r = atom->atom;
  std::free(atom);
  return r;
}

String Xorg::get_win_text_property(xcb_window_t win, xcb_atom_t atom){
  String text;

  xcb_get_property_reply_t *text_prop = get_win_property(win, atom);
//  ERR_FAIL_COND_MSG(text_prop == NULL, vformat("cannot get string property of %d", win));
  ERR_FAIL_COND_V(text_prop == NULL, text);

  const char *p_val = (const char *)xcb_get_property_value(text_prop);
  uint32_t len = xcb_get_property_value_length(text_prop);
  char buf[len+1];
  memcpy(buf, p_val, len);
  buf[len] = 0;
  text.parse_utf8(buf, 1024);
  std::free(text_prop);

  return text;
}

xcb_get_property_reply_t* Xorg::get_win_property(xcb_window_t win, xcb_atom_t atom){
  xcb_generic_error_t *err = NULL;

  xcb_get_property_reply_t *prop =
    xcb_get_property_reply(
      conn,
      xcb_get_property(conn, 0, win, atom, 0, 0, 4096),
      &err);

  if (err != NULL || xcb_get_property_value_length(prop) == 0) {
    std::free(prop);
    return NULL;
  }
  return prop;
}

void Xorg::refresh_xorg_windows() {
  xcb_generic_error_t *err = NULL;
  xcb_screen_iterator_t screen_iter =
    xcb_setup_roots_iterator(xcb_get_setup(conn));

  xcb_atom_t ATOM__NET_CLIENT_LIST = get_atom("_NET_CLIENT_LIST");
  xcb_atom_t ATOM__NET_WM_NAME = get_atom("_NET_WM_NAME");
  xcb_atom_t ATOM_WM_CLASS = get_atom("WM_CLASS");

  windows.clear();

  for(; screen_iter.rem > 0; xcb_screen_next(&screen_iter)) {
    xcb_get_property_reply_t *cl_list =
      xcb_get_property_reply(
        conn,
        xcb_get_property(conn, 0, screen_iter.data->root, ATOM__NET_CLIENT_LIST, 0, 0, 4096),
        &err);
    if (err != NULL) {
      std::free(cl_list);
      ERR_FAIL_MSG("failed to get property reply");
    }

    uint32_t len = xcb_get_property_value_length(cl_list) / sizeof(xcb_window_t);
    for (uint32_t i = 0; i < len; i++) {
      Ref<XorgWindowInfo> xw;
      xw.instantiate();

      xcb_window_t win = ((xcb_window_t *)xcb_get_property_value(cl_list))[i];
      xw->set_wm_name(get_win_text_property(win, ATOM__NET_WM_NAME));
      xw->set_wm_class(get_win_text_property(win, ATOM_WM_CLASS));

      xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
          conn,
          xcb_get_geometry(conn, win),
          &err);
      ERR_FAIL_COND(err != NULL);

      xcb_translate_coordinates_reply_t* trans_coord =
        xcb_translate_coordinates_reply(
          conn,
          xcb_translate_coordinates(conn, win, geom->root, geom->x, geom->y),
          &err);
      ERR_FAIL_COND(err != NULL);
      Rect2i r = Rect2i(
          geom->x, geom->y, geom->width, geom->height);

      xw->set_wm_rect(r);
      /*
      printf("\tTransXY: (%d, %d)\n\tGeom (x,y) (w,h) (b): (%d, %d) (%d, %d) (%d)\n",
      trans_coord->dst_x, trans_coord->dst_y,
      geom->x, geom->y, geom->width, geom->height, geom->border_width);
      */

      std::free(geom);
      std::free(trans_coord);

      windows.push_back(xw);
    }
  }
}
