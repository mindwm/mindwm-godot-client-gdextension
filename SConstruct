#!/usr/bin/env python
import os
import sys

env = Environment(tools=["default"], PLATFORM="")
env.PrependENVPath("PATH", os.getenv("PATH"))
profile = ARGUMENTS.get("profile", "")
scons_cache_path = os.environ.get("SCONS_CACHE")
env["platform"] = ARGUMENTS.get('platform')
env["target"] = ARGUMENTS.get("target")
env["arch"] = "x86_64"
if env["target"].endswith("_debug"):
  env["mode"] = "debug"
  env["debug_symbols"] = True
  env.dev_build = True
  env["optimize"] = "debug"
else:
  env["mode"] = "release"
  env["debug_symbols"] = False
  env.dev_build = False
  env["optimize"] = "speed"

env.use_hot_reload = True
env["suffix"] = ".{}.{}.{}".format(
  env["platform"],
  env["target"],
  env["arch"]
  )

env.Append(CXXFLAGS=["-std=c++17"])
env.Append(CXXFLAGS=["-fno-exceptions"])
#env.Append(CCFLAGS=["-fvisibility=default"])
#env.Append(LINKFLAGS=["-fvisibility=default"])
env.Append(CCFLAGS=["-fvisibility=hidden"])
env.Append(LINKFLAGS=["-fvisibility=hidden"])

if env["debug_symbols"]:
    env.Append(CCFLAGS=["-gdwarf-4"])
    if env.dev_build:
        env.Append(CCFLAGS=["-g3"])
    else:
        env.Append(CCFLAGS=["-g2"])
else:
    env.Append(LINKFLAGS=["-s"])

if env["optimize"] == "speed":
    env.Append(CCFLAGS=["-O3"])
elif env["optimize"] == "speed_trace":
    env.Append(CCFLAGS=["-O2"])
elif env["optimize"] == "size":
    env.Append(CCFLAGS=["-Os"])
elif env["optimize"] == "debug":
    env.Append(CCFLAGS=["-Og"])
elif env["optimize"] == "none":
    env.Append(CCFLAGS=["-O0"])

if env.use_hot_reload:
    env.Append(CXXFLAGS=["-fno-gnu-unique"])

env.Append(CCFLAGS=["-fPIC", "-Wwrite-strings"])
env.Append(LINKFLAGS=["-Wl,-R,'$$ORIGIN'"])
env.Append(LINKFLAGS=["-Lgodot-cpp/bin/"])

if env["arch"] == "x86_64":
    env.Append(CCFLAGS=["-m64", "-march=x86-64"])
    env.Append(LINKFLAGS=["-m64", "-march=x86-64"])

env.Append(CPPDEFINES=["LINUX_ENABLED", "UNIX_ENABLED"])

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env["ENV"] = os.environ
env.Append(CPPPATH=["src/", "godot-cpp/include/", "godot-cpp/gen/include/", "godot-cpp/gdextension/" ])
env.Append(LIBS=
    [ 'X11'
    , 'X11-xcb'
    , 'xcb'
    , 'xcb-composite'
    , 'xcb-image'
    , 'xcb-randr'
    , 'xcb-xfixes'
    , "godot-cpp.linux.template_{}.x86_64.a".format(env["mode"])
    ])
sources = Glob("src/*.cpp")

if env["platform"] == "linux":
    library = env.SharedLibrary(
        "demo/bin/libxorg{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )
else:
    library = None

Default(library)
