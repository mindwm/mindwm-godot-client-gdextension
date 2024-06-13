#include "XorgWindowTexture.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void XorgWindowTexture::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_device_texture_id"), &XorgWindowTexture::get_device_texture_id);
}

uint64_t XorgWindowTexture::get_device_texture_id() {
  return _internal_texture_id;
}

XorgWindowTexture::XorgWindowTexture() {
  const RID rid = get_rid();
  _internal_texture_id = RenderingServer::get_singleton()->texture_get_native_handle(rid);
  UtilityFunctions::print(vformat("_internal_texture: %s (%s)", rid, _internal_texture_id));
}

XorgWindowTexture::~XorgWindowTexture() {
}
