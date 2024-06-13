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

protected:
  static void _bind_methods();

public:
  XorgWindowTexture();
  ~XorgWindowTexture();
};
}

#endif // XORG_WINDOW_TEXTURE_H
