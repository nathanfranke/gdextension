#!/usr/bin/env python
import os
from glob import glob
from pathlib import Path

# TODO: Do not copy environment after godot-cpp/test is updated <https://github.com/godotengine/godot-cpp/blob/master/test/SConstruct>.
env = SConscript("godot-cpp/SConstruct")

# Add source files.
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

# Find gdextension path even if the directory or extension is renamed.
# Example: project/addons/example/example.gdextension
(extension_path,) = glob("project/addons/*/*.gdextension")

# Find the addon path (e.g. project/addons/example).
addon_path = Path(extension_path).parent

# Find the extension name from the gdextension file (e.g. example).
extension_name = Path(extension_path).stem

# TODO: Cache is disabled currently.
# scons_cache_path = os.environ.get("SCONS_CACHE")
# if scons_cache_path != None:
#     CacheDir(scons_cache_path)
#     print("Scons cache enabled... (path: '" + scons_cache_path + "')")

# Create the library target (e.g. libexample.linux.debug.x86_64.so).
if env["platform"] == "osx":
    library = env.SharedLibrary(
        "{}/bin/lib{}.{}.{}.framework/{1}.{2}.{3}".format(
            addon_path,
            extension_name,
            env["platform"],
            env["target"],
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/bin/lib{}.{}.{}.{}{}".format(
            addon_path,
            extension_name,
            env["platform"],
            env["target"],
            env["arch_suffix"],
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(library)
