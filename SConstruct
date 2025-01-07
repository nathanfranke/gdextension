#!/usr/bin/env python
import os
import subprocess
from glob import glob
from pathlib import Path

# TODO: Do not copy environment after godot-cpp/test is updated <https://github.com/godotengine/godot-cpp/blob/master/test/SConstruct>.
env = SConscript("godot-cpp/SConstruct")

# Add source files.
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

# Add chuck files
env.Append(CPPPATH=["chuck/src/core/"])
sources+= Glob("chuck/src/core/*.cpp")

# Need c files in core as well as core/lo
sources += Glob("chuck/src/core/*.c", exclude=["chuck/src/core/chuck_yacc.c"])

# Exclude server_thread.c to get around 'No threading implementation available' compile error on Windows. Unsure for other platforms.
# For some reason server_thread.c isn't included in the Visual Studio solution, so the error doesn't happen when building Chuck directly.
sources += Glob("chuck/src/core/lo/*.c")

# These external libraries are required for the Windows build.
# env.Append(LIBS=["winmm","dsound","dinput8","dxguid","wsock32","ws2_32","iphlpapi","user32","advapi32"])

env.Append(CXXFLAGS=['-fexceptions'])

# Function to run bison
def run_bison(target, source, env):
    subprocess.run(['bison', '-dv', '-b', 'chuck', 'chuck/src/core/chuck.y'], check=True)

# Function to run flex
def run_flex(target, source, env):
    subprocess.run(['flex', '-Ichuck/src/core', '-ochuck.yy.c', 'chuck/src/core/chuck.lex'], check=True)

# Add custom builders for bison and flex
bison_builder = Builder(action=run_bison)
flex_builder = Builder(action=run_flex)

env.Append(BUILDERS={'Bison': bison_builder, 'Flex': flex_builder})

# Generate chuck.tab.h and chuck.tab.c using bison
bison_output = env.Bison(target=['chuck.tab.c', 'chuck.tab.h'], source='chuck/src/core/chuck.y')

# Generate chuck.yy.c using flex
flex_output = env.Flex(target='chuck.yy.c', source='chuck/src/core/chuck.lex')
env.Depends(flex_output, bison_output)

# Add the generated files to the sources list
sources += ['chuck.tab.c', 'chuck.yy.c']

# Add the directory containing chuck.tab.h to the include path
env.Append(CPPPATH=['.'])

# Find gdextension path even if the directory or extension is renamed (e.g. project/addons/example/example.gdextension).
(extension_path,) = glob("project/addons/*/*.gdextension")

# Get the addon path (e.g. project/addons/example).
addon_path = Path(extension_path).parent

# Get the project name from the gdextension file (e.g. example).
project_name = Path(extension_path).stem

scons_cache_path = os.environ.get("SCONS_CACHE")
if scons_cache_path != None:
    CacheDir(scons_cache_path)
    print("Scons cache enabled... (path: '" + scons_cache_path + "')")

# Create the library target (e.g. libexample.linux.debug.x86_64.so).
debug_or_release = "release" if env["target"] == "template_release" else "debug"
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/bin/lib{1}.{2}.{3}.framework/{1}.{2}.{3}".format(
            addon_path,
            project_name,
            env["platform"],
            debug_or_release,
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/bin/lib{}.{}.{}.{}{}".format(
            addon_path,
            project_name,
            env["platform"],
            debug_or_release,
            env["arch"],
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(library)
