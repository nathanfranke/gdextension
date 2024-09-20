# gdextension

GDExtension template that automatically builds into a self-contained addon for the Godot Asset Library.

## Getting started

1. Clone this repository (or a new repository with this template) with submodules.
    - `git clone --recurse-submodules https://github.com/nathanfranke/gdextension.git`
    - `cd gdextension`
2. Update to the latest `godot-cpp`.
    - `git submodule update --remote`
3. Install VCPKG <https://learn.microsoft.com/en-us/vcpkg/get_started/overview>
4. Modify the VCPKG dependence and install.
    - `vcpkg install`
5. Build a debug binary for the current platform.
    - `scons`
6. Import, edit, and play `project/` using Godot Engine 4+.
    - `godot --path project/`
7. Check the output:

```text
Hello GDScript!
Hello GDExtension Node!
Hello GDExtension Singleton!
```

## Repository structure

- `project/` - Godot project boilerplate.
  - `addons/example/` - Files to be distributed to other projects.¹
  - `demo/` - Scenes and scripts for internal testing. Not strictly necessary.
- `src/` - Source code of this extension.
- `godot-cpp/` - Submodule needed for GDExtension compilation.

¹ Before distributing as an addon, all binaries for all platforms must be built and copied to the `bin/` directory. This is done automatically by GitHub Actions.

## Make it your own

1. Rename `project/addons/example/` and `project/addons/example/example.gdextension`.
2. Replace `LICENSE`, `README.md`, and your code in `src/`.
3. Not required, but consider leaving a note about this template if you found it helpful!

## Distributing your extension on the Godot Asset Library with GitHub Actions

1. Go to Repository→Actions and download the latest artifact.
2. Test the artifact by extracting the addon into a project.
3. Create a new release on GitHub, uploading the artifact as an asset.
4. On the asset, Right Click→Copy Link to get a direct file URL. Don't use the artifacts directly from GitHub Actions, as they expire.
5. When submitting/updating on the Godot Asset Library, Change "Repository host" to `Custom` and "Download URL" to the one you copied.
