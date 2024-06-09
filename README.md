# Overview
Right now it works only under Linux with Xorg server.

;TBD

# Build
Update `godot-cpp` submodule
```sh
git submodule update --init
```
then build gdextension via
```sh
scons platform=linux target=template_release && scons platform=linux target=template_debug
```

# Demo scene
Just run Godot 4.3+, import `demo/project.godot` and run as usual.
