{ pkgs ? builtins.currentSystem }:
with pkgs;
let
  inherit (pkgs);
  libs = [ scons libpulseaudio freetype openssl alsaLib libGLU zlib yasm vulkan-loader libGL
  ] ++ (with xorg; [
	libX11
	libXcursor
	libXinerama
	libXrandr
	libXrender
	libXi
	libXext
	libXfixes
	libxcb
	xcbutilerrors
	xcbutilimage
	xcbutilrenderutil
	xcbutilwm
	]);
in
pkgs.stdenv.mkDerivation {
  name = "MindWM Godot Client";
  buildInputs = [ pkg-config ] ++ libs;
  shellHook = ''
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${lib.makeLibraryPath libs}"
    export PKG_CONFIG_PATH="$PKG_CONFIG_PATH_FOR_TARGET"
  '';
}
