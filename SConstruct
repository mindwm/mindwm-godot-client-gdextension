#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env["ENV"] = os.environ
env.Append(CPPPATH=["src/"])
#env.Append(LIBS=['X11', 'Xfixes', 'X11-xcb', 'xcb', 'xcb-composite', 'xcb-image', 'xcb-randr'])
env.Append(LIBS=['X11', 'X11-xcb', 'xcb', 'xcb-composite', 'xcb-image', 'xcb-randr', 'xcb-xfixes'])
sources = Glob("src/*.cpp")

if env["platform"] == "linux":
    library = env.SharedLibrary(
        "demo/bin/libxorg{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )
else:
    library = None

Default(library)
