# gdextension

Self-contained gdextension template for Godot Engine 4.x

### Getting started:
1. Clone this repository (or a new repository with this template) with submodules.
    - `git clone --recurse-submodules https://github.com/nathanfranke/gdextension.git` \
      `cd gdextension`
2. Build all libraries for the current platform.
    - `scons`
3. Import, edit, and play `project/` using Godot Engine (4.x or later).
4. Check the output:
   ```
   Hello GDScript!
   Hello GDExtension Node!
   Hello GDExtension Singleton!
   ```

### Repository structure:
- `project/` - Godot project template.
  - `addons/example/` - Files to be distributed to other projects.¹
  - `demo/` - Scenes and scripts for internal testing. Not strictly necessary.
- `src/` - Source code of this extension.
- `godot-cpp/` - Submodule needed for GDExtension compilation.

¹ Before distributing as an addon, all platform binaries must be in the `bin/` directory. This is done automatically by GitHub Actions.

### Make it your own:
1. Rename `project/addons/example/` and `project/addons/example/example.gdextension`.
2. Replace `LICENSE` and `README.md`.
3. Not required, but consider leaving a note about this template if you found it helpful.

### Distributing your extension on the Godot Asset Library:
1. Go to Repository→Actions→Latest Action and download the artifact.
2. Test the artifact (it should be a working Godot project).
3. Create a new release on GitHub, uploading the artifact as an asset.
4. On the asset, Right Click→Copy Link to get a direct file URL.
5. When submitting/updating on the Godot Asset Library, Change `Repository host` to `Custom` and `Download URL` to the one you copied.
