#!/usr/bin/env python
from glob import glob
from pathlib import Path

# TODO: Do not copy environment after godot-cpp/test is updated <https://github.com/godotengine/godot-cpp/blob/master/test/SConstruct>.
env = SConscript("godot-cpp/SConstruct")

# Add source files.
env.Append(CPPPATH=["src/"])

vcpkg_triplet = {
    'windows': 'x64-windows',
    'linux': 'x64-linux',
    'macos': 'arm64-osx'
}[env["platform"]]

# Add vcpkg include paths.
env.Append(CPPPATH=[f"vcpkg_installed/{vcpkg_triplet}/include"])

# Add vcpkg library paths.
env.Append(LIBPATH=[f"vcpkg_installed/{vcpkg_triplet}/lib"])

# Add vcpkg libraries.
env.Append(LIBS=[lib.stem for lib in Path(
    f"vcpkg_installed/{vcpkg_triplet}/lib").glob("*.a")])

sources = Glob("src/*.cpp")

# Find gdextension path even if the directory or extension is renamed (e.g. project/addons/example/example.gdextension).
(extension_path,) = glob("project/addons/*/*.gdextension")

# Find the addon path (e.g. project/addons/example).
addon_path = Path(extension_path).parent

# Find the project name from the gdextension file (e.g. example).
project_name = Path(extension_path).stem

# TODO: Cache is disabled currently.
# scons_cache_path = os.environ.get("SCONS_CACHE")
# if scons_cache_path != None:
#     CacheDir(scons_cache_path)
#     print("Scons cache enabled... (path: '" + scons_cache_path + "')")

# Create the library target (e.g. libexample.linux.debug.x86_64.so).
debug_or_release = "release" if env["target"] == "template_release" else "debug"
if env["platform"] == "macos":
    library = env.SharedLibrary(
        f"{addon_path}/bin/lib{project_name}.{env['platform']}.{debug_or_release}.framework/{project_name}.{env['platform']}.{debug_or_release}",
        source=sources,
    )
else:
    library = env.SharedLibrary(
        f"{addon_path}/bin/lib{project_name}.{env['platform']}.{debug_or_release}.{env['arch']}{env['SHLIBSUFFIX']}",
        source=sources,
    )

Default(library)
