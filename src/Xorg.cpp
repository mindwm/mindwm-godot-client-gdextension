#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/color_names.inc.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <xcb/xcb.h>
#include <xcb/composite.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_image.h>
#include <xcb/xfixes.h>
#include "Xorg.h"

using namespace godot;

void Xorg::_bind_methods() {
  ClassDB::bind_method(D_METHOD("init"), &Xorg::init);
  ClassDB::bind_method(D_METHOD("list_windows"), &Xorg::list_windows);
  ClassDB::bind_method(D_METHOD("refresh_xorg_windows"), &Xorg::refresh_xorg_windows);
  ClassDB::bind_method(D_METHOD("get_wm_window", "p_index"), &Xorg::get_wm_window);
  ClassDB::bind_method(D_METHOD("get_wm_window_texture", "p_index"), &Xorg::get_wm_window_texture);
  ClassDB::bind_method(D_METHOD("capture_window", "p_index"), &Xorg::capture_window);
  ClassDB::bind_method(D_METHOD("set_NET_WM_DESKTOP", "p_index", "value"), &Xorg::set_net_wm_desktop);
  // signals
  ADD_SIGNAL(MethodInfo("window_created", PropertyInfo(Variant::OBJECT, "window_info")));
  ADD_SIGNAL(MethodInfo("window_destroyed", PropertyInfo(Variant::OBJECT, "window_info")));
  ADD_SIGNAL(MethodInfo("window_configured", PropertyInfo(Variant::OBJECT, "window_info")));
  ADD_SIGNAL(MethodInfo("clipboard_notify", PropertyInfo(Variant::INT, "window_id")));
}

Xorg::Xorg() {
  // Initialize any variables here.
  time_passed = 0.0;
}

void Xorg::init(){
  conn = xcb_connect(NULL, NULL);
  // TODO: need to check that a compositor is avaliable
  // in other case we cannot capture window content
//  xcomposite_load();
  ERR_FAIL_NULL(conn);
  UtilityFunctions::print("[mindwm]: connected to X11 server");
  UtilityFunctions::print("[mindwm]: xcomposite initialized");
}

Xorg::~Xorg() {
  // Add your cleanup here.
}

void Xorg::_ready() {
  UtilityFunctions::print("[mindwm]: Xorg module initialized");
}

void Xorg::set_net_wm_desktop(int p_index, uint64_t val) {
  ERR_FAIL_INDEX(p_index, windows.size());

  xcb_atom_t ATOM__NET_WM_DESKTOP = get_atom("_NET_WM_DESKTOP");
  xcb_atom_t ATOM__NET_WM_STATE = get_atom("_NET_WM_STATE");
  xcb_atom_t ATOM__NET_WM_STATE_STICKY = get_atom("_NET_WM_STATE_STICKY");
//  xcb_atom_t ATOM__NET_WM_STATE_ABOVE = get_atom("_NET_WM_STATE_ABOVE");
//  xcb_atom_t ATOM__NET_WM_STATE_SKIP_TASKBAR = get_atom("_NET_WM_STATE_SKIP_TASKBAR");
//  xcb_atom_t ATOM__NET_WM_STATE_SKIP_PAGER = get_atom("_NET_WM_STATE_SKIP_PAGER");
  xcb_atom_t ATOM__NET_WM_WINDOW_TYPE = get_atom("_NET_WM_WINDOW_TYPE");
  xcb_atom_t ATOM__NET_WM_WINDOW_TYPE_DOCK = get_atom("_NET_WM_WINDOW_TYPE_DOCK");

  xcb_change_property(conn,
    XCB_PROP_MODE_REPLACE,
    windows[p_index]->get_win_id(),
    ATOM__NET_WM_WINDOW_TYPE,
    XCB_ATOM_ATOM,
    32,
    1,
    &ATOM__NET_WM_WINDOW_TYPE_DOCK);

  uint32_t data[] = {0xFFFFFFFF};
  xcb_change_property(conn,
    XCB_PROP_MODE_REPLACE,
    windows[p_index]->get_win_id(),
    ATOM__NET_WM_DESKTOP,
    XCB_ATOM_CARDINAL,
    32,
    1,
    (const void*)data);

  uint32_t states[] = {
    ATOM__NET_WM_STATE_STICKY,
 //   ATOM__NET_WM_STATE_ABOVE,
 //   ATOM__NET_WM_STATE_SKIP_TASKBAR,
 //   ATOM__NET_WM_STATE_SKIP_PAGER
  };
  xcb_change_property(conn,
    XCB_PROP_MODE_REPLACE,
    windows[p_index]->get_win_id(),
    ATOM__NET_WM_STATE,
    XCB_ATOM_ATOM,
    32,
    1,
    (const void*) states);

  xcb_flush(conn);
}

Ref<XorgWindowInfo> Xorg::get_wm_window(int p_index) {
  ERR_FAIL_INDEX_V(p_index, windows.size(), nullptr);

  return windows[p_index];
}

Ref<XorgWindowTexture> Xorg::get_wm_window_texture(int p_index) {
  ERR_FAIL_INDEX_V(p_index, window_textures.size(), nullptr);

  return window_textures[p_index];
}

Ref<XorgWindowInfo> Xorg::find_window_by_id(xcb_window_t win_id) {
  Ref<XorgWindowInfo> res;
  for (Ref<XorgWindowInfo> &E : windows) {
    if (E->get_win_id() == win_id) {
      res = E;
      break;
    }
  }
  return res;
}

Ref<XorgWindowInfo> Xorg::find_window_by_parent_id(xcb_window_t parent_id) {
  Ref<XorgWindowInfo> res;
  for (Ref<XorgWindowInfo> &E : windows) {
    if (E->get_parent_id() == parent_id) {
      res = E;
      break;
    }
  }
  return res;
}

TypedArray<XorgWindowInfo> Xorg::list_windows() {
  TypedArray<XorgWindowInfo> ws;
  ws.resize(windows.size());

  for (int i = 0; i < windows.size(); i++) {
    ws[i] = get_wm_window(i);
  }

  return ws;
}

void Xorg::_notification(int p_what) {
  // Don't run if we're in the editor
  UtilityFunctions::print(vformat("_notification: %s", p_what));
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  switch (p_what) {
//  case NOTIFICATION_READY: {
  case NOTIFICATION_POSTINITIALIZE: {
    eventsWatcherTerminated = false;
    clipboardWatcherTerminated = false;
    eventsWatcher.instantiate();
    UtilityFunctions::print(vformat("starting eventWatcher thread"));
    eventsWatcher->start(callable_mp(this, &Xorg::watchEvents), Thread::PRIORITY_NORMAL);
    UtilityFunctions::print(vformat("starting clipboardWatcher thread"));
    clipboardWatcher.instantiate();
    clipboardWatcher->start(callable_mp(this, &Xorg::watchClipboard), Thread::PRIORITY_NORMAL);
  } break;
  case NOTIFICATION_WM_CLOSE_REQUEST: {
    eventsWatcherTerminated = true;
    clipboardWatcherTerminated = true;
    UtilityFunctions::print(vformat("set terminate flag for all watchers"));
    // NOTE: we need to send a dummy event to the root window
    // to unblock the watcher thread and allow it to finish the tasks
    send_xorg_dummy_event();
    if (eventsWatcher.is_valid()) {
      eventsWatcher->wait_to_finish();
      clipboardWatcher->wait_to_finish();
    }

    eventsWatcher.unref();
    clipboardWatcher.unref();
  };
  }
}

void Xorg::_process(double delta) {
  time_passed += delta;
}

xcb_atom_t Xorg::intern_atom(const char *str)
{
    xcb_intern_atom_reply_t *rep;
    xcb_atom_t atom;
    uint16_t name_len = strlen(str);
    rep = xcb_intern_atom_reply(conn, xcb_intern_atom(conn, 0, name_len, str), NULL);
    atom = rep->atom;
    std::free(rep);
    return atom;
}

void Xorg::watchClipboard() {
  UtilityFunctions::print(vformat("starting clipboard events watcher"));
  xcb_connection_t *conn;
  conn = xcb_connect(NULL, NULL);
  xcb_screen_t *screen;
  xcb_window_t event_window = xcb_generate_id(conn);
  xcb_generic_event_t *event;
  xcb_void_cookie_t cookie;

  screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

  const xcb_query_extension_reply_t *query_ext = xcb_get_extension_data(conn, &xcb_xfixes_id);

  uint32_t flags[] =  { XCB_EVENT_MASK_PROPERTY_CHANGE };
  xcb_create_window(conn, screen->root_depth, event_window, screen->root,
    -1, -1, 1, 1, 0, XCB_COPY_FROM_PARENT,
    screen->root_visual, XCB_CW_EVENT_MASK,
    flags);
  xcb_discard_reply(conn, xcb_xfixes_query_version(conn, 1, 0).sequence);

  xcb_atom_t selection = intern_atom("CLIPBOARD");
  xcb_xfixes_select_selection_input(conn, event_window, selection,
            XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER |
            XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_WINDOW_DESTROY |
            XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_CLIENT_CLOSE);

  xcb_flush(conn);

  while ((event = xcb_wait_for_event(conn))) {
    UtilityFunctions::print(vformat("[clipboardWatcher]: new event: 0x%x", event->response_type & ~0x7f));
    if (clipboardWatcherTerminated)
      break;

    uint8_t type = event->response_type & 0x7f;
//    UtilityFunctions::print(vformat("[clipboardWatcher]: event type: 0x%x", type));
    if (query_ext != NULL && type == query_ext->first_event + XCB_XFIXES_SELECTION_NOTIFY) {
      xcb_xfixes_selection_notify_event_t *ev = (xcb_xfixes_selection_notify_event_t *) event;
//      UtilityFunctions::print(vformat("[clipboardWatcher]: Selection changed, new owner is 0x%x\n", ev->owner));
      /*
      Ref<XorgWindowInfo> win;
      win = find_window_by_id(ev->owner);
      if (!win.is_valid())
        win = find_window_by_parent_id(ev->owner);

      ERR_FAIL_COND_MSG(!win.is_valid(), vformat("failed to find XorgWindowInfo for 0x%x", ev->owner));
      */
      call_deferred("emit_signal", "clipboard_notify", ev->owner);
    }
    else {
      switch (type) {
        case 0:
          {
            xcb_generic_error_t *generr = (xcb_generic_error_t *) event;
            UtilityFunctions::print(vformat("[clipboardWatcher]: Got error %d from request %d:%d\n",
            generr->error_code, generr->major_code,
            generr->minor_code));
          }
          break;
        case XCB_SELECTION_NOTIFY:
          {
      	    call_deferred("emit_signal", "clipboard_notify");
  	      }
          break;
        default:
          {
            UtilityFunctions::print(vformat("[clipboardWatcher]: Got unexpected event %d\n", event->response_type));
          }
      }
    }
    std::free(event);
    xcb_flush(conn);
  }
  xcb_disconnect(conn);
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
        (const uint32_t[])
        { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        }
        );

  xcb_flush(conn);

  ERR_FAIL_COND(xcb_request_check(conn, cookie));

  while ((event = xcb_wait_for_event(conn))) {
    UtilityFunctions::print(vformat("[eventsWatcher]: new event 0x%x", event->response_type & ~0x80));
    if (eventsWatcherTerminated)
      break;

    switch (event->response_type & ~0x80) {
      case XCB_CREATE_NOTIFY:
        {
        xcb_create_notify_event_t *create_event = (xcb_create_notify_event_t *)event;
        if (create_event->width > 1 || create_event->height > 1) {
          UtilityFunctions::print(vformat("[eventsWatcher]: new window event: 0x%08x (0x%08x)", create_event->window, create_event->parent));
          UtilityFunctions::print(vformat("[eventsWatcher]: \tWxH (%dx%d)", create_event->width, create_event->height));
          add_window(create_event->window, create_event->parent);
        }
        else {
          // maybe some utility windows
        }
        break;
        }
      case XCB_DESTROY_NOTIFY:
        {
        xcb_destroy_notify_event_t *destroy_event = (xcb_destroy_notify_event_t *)event;
        UtilityFunctions::print(vformat("[eventsWatcher]: destroy window event: 0x%08x", destroy_event->window));
        Ref<XorgWindowInfo> w;
        w = find_window_by_id(destroy_event->window);
        if (w != NULL) {
          UtilityFunctions::print(vformat("[eventsWatcher]: destroy by win_id 0x%x", w->get_win_id()));
          remove_window(w);
        }
        else {
          w = find_window_by_parent_id(destroy_event->window);
          if (w != NULL) {
            UtilityFunctions::print(vformat("[eventsWatcher]: destroy by parent_id 0x%x", w->get_win_id()));
            remove_window(w);
          }
        }
        break;
        }
      case XCB_REPARENT_NOTIFY:
        {
        xcb_reparent_notify_event_t *reparent_event = (xcb_reparent_notify_event_t *)event;
        UtilityFunctions::print(vformat("[eventsWatcher]: reparent window event: 0x%08x new parent 0x%x", reparent_event->window, reparent_event->parent));

        Ref<XorgWindowInfo> w;
        w = find_window_by_id(reparent_event->window);
        if (w != NULL) {
          UtilityFunctions::print(vformat("[eventsWatcher]: set parent: 0x%08x old 0x%x new 0x%x", w->get_win_id(), w->get_parent_id(), reparent_event->parent));
          w->set_parent_id(reparent_event->parent);
        }

        break;
        }
      case XCB_CONFIGURE_NOTIFY:
        {
        xcb_configure_notify_event_t *configure_event = (xcb_configure_notify_event_t *)event;
        UtilityFunctions::print(vformat("[eventsWatcher]: configure window event: 0x%08x Rect(%d, %d, %d, %d)",
              configure_event->window,
              configure_event->x,
              configure_event->y,
              configure_event->width,
              configure_event->height
              ));

        Ref<XorgWindowInfo> w;
        w = find_window_by_id(configure_event->window);
        if (w == NULL)
          w = find_window_by_parent_id(configure_event->window);
        break;
        }
    }
    std::free(event);
    xcb_flush(conn);
  }
  xcb_disconnect(conn);
}

void Xorg::send_xorg_dummy_event() {
  // TODO: need to send proper event to trigger the
  // eventWatcher thread. Right now it's a placeholder
  // utility_functions
  xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
  xcb_window_t root = screen->root;
  
  // Create a dummy event
  xcb_client_message_event_t event = {
      .response_type = XCB_CLIENT_MESSAGE,
      .format = 32,
      .sequence = 0,
      .window = root,
      .type = XCB_ATOM_NONE,
      .data = {0}
  };
  
  // Send the dummy event to the root window
  xcb_send_event(conn, 0, root, XCB_EVENT_MASK_NO_EVENT, (char *)&event);
  UtilityFunctions::print("[mindwm]: sending dummy event to the root window");
//  xcb_send_event(conn, 0, root, XCB_CW_EVENT_MASK, (char *)&event);
  xcb_flush(conn);
}

xcb_atom_t Xorg::get_atom(const char *atom_name){
  xcb_intern_atom_cookie_t atom_c = xcb_intern_atom(conn, 1, strlen(atom_name), atom_name);
  xcb_intern_atom_reply_t *atom = xcb_intern_atom_reply(conn, atom_c, NULL);
  xcb_atom_t r = atom->atom;
  std::free(atom);
  return r;
}

xcb_window_t Xorg::get_window_parent(xcb_window_t win) {
  xcb_generic_error_t *err = NULL;

  xcb_query_tree_reply_t * rep =
    xcb_query_tree_reply (conn,
      xcb_query_tree(conn, win),
      &err);

  ERR_FAIL_COND_V(err != NULL, -1);
  return rep->parent;
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

void Xorg::remove_window(Ref<XorgWindowInfo> elem) {
  int i = windows.find(elem);
  if (i) {
    // TODO: need to unref the elem here?
    windows.remove_at(i);
    window_textures.remove_at(i);
    call_deferred("emit_signal", "window_destroyed", elem);
  }
}

void Xorg::add_window(xcb_window_t win, xcb_window_t parent) {
  xcb_generic_error_t *err = NULL;
  xcb_atom_t ATOM__NET_WM_NAME = get_atom("_NET_WM_NAME");
  xcb_atom_t ATOM_WM_CLASS = get_atom("WM_CLASS");

  Ref<XorgWindowInfo> xw;
  xw.instantiate();

  xw->set_win_id(win);
  xw->set_parent_id(parent);
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
//      trans_coord->dst_x, trans_coord->dst_y,
      geom->x, geom->y,
      geom->width, geom->height);

  xw->set_wm_rect(r);

  printf("\t(%x) TransXY: (%d, %d)\n\tGeom (x,y) (w,h) (b): (%d, %d) (%d, %d) (%d)\n",
  geom->root, trans_coord->dst_x, trans_coord->dst_y,
  geom->x, geom->y, geom->width, geom->height, geom->border_width);

  std::free(geom);
  std::free(trans_coord);

  windows.push_back(xw);
  Ref<XorgWindowTexture> win_tex;
  win_tex.instantiate();
  window_textures.push_back(win_tex);
//  UtilityFunctions::print(vformat("[mindwm]: Xorg texRID: %s", win_tex->get_rid()));
  call_deferred("emit_signal", "window_created", xw);
}

void Xorg::configure_window(Ref<XorgWindowInfo> w, Rect2i rect) {
  w->set_wm_rect(rect);
  call_deferred("emit_signal", "window_configured", w);
}

void Xorg::refresh_xorg_windows() {
  xcb_generic_error_t *err = NULL;
  xcb_screen_iterator_t screen_iter =
    xcb_setup_roots_iterator(xcb_get_setup(conn));

  xcb_atom_t ATOM__NET_CLIENT_LIST = get_atom("_NET_CLIENT_LIST");
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
      xcb_window_t win = ((xcb_window_t *)xcb_get_property_value(cl_list))[i];
      xcb_window_t parent = get_window_parent(win);
      add_window(win, parent);
    }
  }
}

xcb_screen_t *xcb_get_screen(xcb_connection_t *xcb, int screen) {
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator(xcb_get_setup(xcb));
  for (; iter.rem; --screen, xcb_screen_next(&iter)) {
    if (screen == 0)
      return iter.data;
  }

  return NULL;
}

bool Xorg::xcomp_check_ewmh(xcb_window_t root) {
  xcb_atom_t ATOM__NET_SUPPORTING_WM_CHECK = get_atom("_NET_SUPPORTING_WM_CHECK");

  xcb_get_property_reply_t *check =
    get_win_property(root, ATOM__NET_SUPPORTING_WM_CHECK);
  if (!check)
    return false;
   
  xcb_window_t ewmh_window =
    ((xcb_window_t *)xcb_get_property_value(check))[0];
  std::free(check);
   
  xcb_get_property_reply_t *check2 = get_win_property(
    ewmh_window, ATOM__NET_SUPPORTING_WM_CHECK);
  if (!check2)
    return false;
  std::free(check2);
   
  return true;
}

void Xorg::capture_window(int p_window_index) {
  Ref<XorgWindowInfo> w = get_wm_window(p_window_index);
  Ref<XorgWindowTexture> tex = get_wm_window_texture(p_window_index);
  Rect2i r = w->get_wm_rect();

  xcb_composite_redirect_window(conn, w->get_win_id(), XCB_COMPOSITE_REDIRECT_AUTOMATIC);
   
  // get the image
  xcb_image_t *wm_image = xcb_image_get(conn, w->get_win_id(), 0, 0, r.size.x, r.size.y, 0xFFFFFFFF, XCB_IMAGE_FORMAT_Z_PIXMAP);
   
  ERR_FAIL_NULL(wm_image);
  Ref<Image> img;
  img.instantiate();
  PackedByteArray pba = PackedByteArray();
  pba.resize(r.size.x * r.size.y * 4);
  memcpy(pba.ptrw(), wm_image->data, r.size.x * r.size.y * 4);
  xcb_image_destroy(wm_image);
  img->set_data(r.size.x, r.size.y, 0, Image::FORMAT_RGBA8, pba);
  tex->set_size_override(r.size);
  tex->set_image(img);
}
