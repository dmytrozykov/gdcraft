# gdcraft

A voxel sandbox game built with Godot 4.6, inspired by Minecraft.

## Building the GDExtension

**Prerequisites:** SCons, a C++ compiler (MSVC on Windows, GCC/Clang on Linux/macOS), Python 3.

```sh
# First-time setup: fetch godot-cpp submodule
git submodule update --init --recursive

# Debug build (used during development)
scons

# Release build
scons target=template_release
```

The compiled library is placed in `project/bin/<platform>/`. Open the `project/` folder in Godot after a successful build.

## License

MIT
