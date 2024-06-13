#ifndef XORG_WINDOW_TEXTURE_H
#define XORG_WINDOW_TEXTURE_H

//#include <godot_cpp/classes/texture2drd.hpp>
//#include <godot_cpp/classes/texture_layered.hpp>
#include <godot_cpp/classes/image_texture.hpp>

namespace godot {

class XorgWindowTexture : public ImageTexture {
  GDCLASS(XorgWindowTexture, ImageTexture);

private:
//  mutable RID texture;
  uint64_t _internal_texture_id;

protected:
  static void _bind_methods();

public:
  uint64_t get_device_texture_id();

  XorgWindowTexture();
  ~XorgWindowTexture();
};
}

#endif // XORG_WINDOW_TEXTURE_H
