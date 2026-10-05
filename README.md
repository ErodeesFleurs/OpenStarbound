# OpenStarbound

<details>
<summary><b>What is this?</b></summary>
 
By a truly unbelievable coincidence, I was out for a walk when I saw a small package fall off a truck ahead of me. Inside, I found the 1.4.4 [**Starbound source code**](https://archive.org/details/starbound_source_code).
**OpenStarbound** is derived from that source code. It fixes many bugs, adds many new features and improves performance. <sup>there is only so much you can do to polish a turd, just play Terraria or something</sup>

**Q:** Doesn't 'Open{game}' imply an engine reversed from scratch? (see Open.. [TTD](https://github.com/OpenTTD/OpenTTD), [RCT2](https://github.com/OpenRCT2/OpenRCT2), [MW](https://gitlab.com/OpenMW/openmw))

**A:** yeah too late to change it now though. whoops


</details>

You must own Starbound for the game assets. The code is worked on whenever I rarely feel like it. Contributions are welcome!

## Installation
### Download the [latest stable release](https://github.com/OpenStarbound/OpenStarbound/releases/latest).
The latest stable release is recommended, as the nightly build can have bugs.

At the moment, you must copy the game assets (**packed.pak**) from your normal Starbound install to the OpenStarbound assets directory before playing. Optionally copy the `user` folder in the same directory as `packed.pak` to move the vanilla playable instrument songs over.

OpenStarbound is a separate installation/executable than Starbound. You can copy your `storage` folder from Starbound to transfer your save data and settings. Launching OpenStarbound with Steam open will load your subscribed Steam mods. If you have any locally installed non-Steam mods, you may want to copy the `mods` folder as well.

An installer is available for Windows. otherwise, extract the client/server zip for your platform and copy the game assets (packed.pak) to the OpenStarbound assets folder.

</details>
<details>
<summary><b>Nightly Builds</b></summary>
 
These link directly to the latest build from the [Actions](https://github.com/OpenStarbound/OpenStarbound/actions?query=branch%3Amain) tab, main branch.
 
**Windows**
[Installer](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-Windows-Installer.zip),
[Client](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-Windows-Client.zip),
[Server](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-Windows-Server.zip)

### **notice: get Linux and macOS builds directly from the [Actions](https://github.com/OpenStarbound/OpenStarbound/actions?query=branch%3Amain) tab until the nightly.link service fixes things**

**Linux**
~~[Client](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-Linux-Clang-Client.tar.lz),~~
~~[Server](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-Linux-Clang-Server.tar.lz)~~

**macOS**
~~[Intel](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-macOS-Intel-Client.tar.lz),~~
~~[ARM](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main/OpenStarbound-macOS-Silicon-Client.tar.lz)~~

---

[All Nightly Builds](https://nightly.link/OpenStarbound/OpenStarbound/workflows/build/main)
</details>

## Changes
Note: Mods that use StarExtensions features often work with OpenStarbound, StarExtensions is deprecated.

### Lighting
**The lightmap generation has been moved off the main thread, and supports higher color range.**
  * Point lights are now additive, which is more accurate - you'll notice that different lights mix together better!
  * Object spread lights are auto-converted to a hybrid light which is 25% additive.

### Assets
* Assets can now run Lua scripts on load, and after all sources have been loaded.
  * These scripts can modify, read, patch and create new assets!
* Lua patch files now exist - **.patch.lua**
  * These can patch JSON assets, as well as images!
### Commands
**View OpenStarbound commands with `/help`! You can also view them [here](https://github.com/OpenStarbound/OpenStarbound/blob/main/assets/opensb/help.config.patch)**
  * Changes to vanilla commands:
    * `/settileprotection`
      * You can now specify as many dungeon IDs as you want: `/settileprotection 69 420 false`
      * You can now specify a range: /settileprotection 0..65535 true
    * `/admin`
      * You can now admin other players: `/admin playerSpecifier` (requires OpenSB server)
### Bug Fixes
* Fixed C++20 shared-pointer builds on Windows and Ubuntu 22.04, and static vcpkg library link order with C++20 modules.
* Invalid character inventories are updated when loading in, allowing players to swap inventory mods with pre-existing characters.
* Fix vanilla world file size bloating issue.
* Modifying a single status property no longer re-networks every status property on the entity (server and client must be running at least OpenStarbound 0.15)
### Misc
* Player functions for saving/loading, modifying the humanoid identity, manipulating the inventory. [Documentation](https://github.com/OpenStarbound/OpenStarbound/tree/main/doc/lua/openstarbound)
* Character swapping (rewrite from StarExtensions, currently command-only: `/swap name` case-insensitive, only substring required)
* Custom user input support with a keybindings menu (rewrite from StarExtensions)
* Positional Voice Chat that works on completely vanilla servers, uses Opus for crisp, HD audio (rewrite from StarExtensions)
  * Both menus are made available in the options menu in this fork rather than as a chat command.
* Multiple font support (switch fonts inline with `^font=name;`, **.ttf** and **.woff2** assets are auto-detected)
  * **.woff2** fonts are much smaller than **.ttf**, [here's a web conversion tool](https://kombu.kanejaku.org/)!
* Experimental changes to the storage of directives in memory to reduce copying - can reduce their impact on frametimes when very long directives are present
  * Works especially well when extremely long directives are used for "vanilla multiplayer-compatible" creations, like [generated clothing](https://silverfeelin.github.io/Starbound-NgOutfitGenerator/) or custom items/objects.
* Perfectly Generic Items will retain the data for what item they were if a mod is uninstalled, and will attempt to restore themselves if re-installed.
* Musical instruments have their own volume slider in the options menu.
* Players can use items while lounging
* Mods can change which scriptPane the Matter Manipulator/Collections sidebar button opens, in [interface.config.patch](https://github.com/OpenStarbound/OpenStarbound/blob/main/assets/opensb/interface.config.patch).
* Items can change their rarity border. [Documentation](https://github.com/OpenStarbound/OpenStarbound/blob/main/doc/json/openstarbound/items.md)

* Client-side tile placement prediction (rewrite from StarExtensions)
  * You can also resize the placement area of tiles on the fly.
* Support for placing foreground tiles with a custom collision type (rewrite from StarExtensions, requires OpenSB server)
  * Additionally, objects can be placed under non-solid foreground tiles.
  * Admin characters have unlimited and unobstructed interaction/placement ranges

* Some minor polish to UI
* The Skybox's sun now matches the system type you're currently in.
  * Previously generated planets will not have this feature and will display the default sun.
  * Modded system types require a patch to display their custom sun.
  * You can also access the skybox sun scale and its default ray colors. For more details see, [sky.config.patch](https://github.com/OpenStarbound/OpenStarbound/blob/main/assets/opensb/sky.config.patch).

**Discord:** reverse the text in the image and you will have the invite code (this is to prevent spam bots)

<details>
<summary>Directives</summary>

 <img width="380" height="70" alt="image" src="https://github.com/user-attachments/assets/9db11a89-0c32-4034-8555-68b54e5918a8" />


</details>

</details>


## Building
Note: Some of these [texts](## "hi :3") are just tooltips rather than links. 

CI caches compiler outputs with sccache and vcpkg binary packages per platform using GitHub Actions cache; changes to the dependency manifests or overlay ports invalidate the vcpkg cache.
The Linux ARM64 Clang ALSA and jemalloc vcpkg overlay ports move files staged under `usr/` or `usr/local/` into the package root before vcpkg validates pkg-config files and imports libraries.
The Linux ARM64 Clang CI also bundles jemalloc's `libjemalloc.so.2` and copyright notice in the raw `dist/` artifact and both client/server archives. Release utilities use `$ORIGIN` to load the library from their executable directory, including when `asset_packer` runs during assembly.
GitHub Actions [limits warning annotations to 10 per step](https://github.com/actions/toolkit/blob/main/docs/problem-matchers.md#limitations), so five platform builds can still show about 50 after fixes expose later diagnostics. Source warnings repeat across platforms; vcpkg/SDL dependency probes and `run-cmake` log-collection messages also contribute to the count. Check individual build logs instead of treating the visible total as the complete warning list.

CMake requires version 3.28 or newer. In-tree compiled targets link `star_build_config` only for project definitions and compiler/linker policy. External usage requirements are grouped by actual consumers: core, core modules, bundled extern code, application, voice, client, and optional Steam tools. Declare source include directories on each target rather than using directory-wide settings. Executables using `$<TARGET_OBJECTS:...>` explicitly link the object code's external groups with `$<LINK_ONLY:...>` and its module libraries; object expressions forward neither usage requirements nor static-library closure.

SDL3, OpenGL, GLEW and Opus discovery is GUI-only. Headless builds retain ImGui and FreeType because the shared Lua implementation exposes ImGui bindings. The Nix ImGui package separates `imgui::imgui` from optional `imgui::imgui_backends`, so non-GUI executables also avoid SDL/OpenGL linkage in a GUI-enabled build; upstream monolithic ImGui packages may still propagate those dependencies. Wayland belongs to SDL's backend dependencies, not a separate OpenStarbound dependency. GLEW uses CMake's standard imported targets without a Nix source rewrite.

The default configurations remain `Debug`, `RelWithAsserts`, `RelWithDebInfo`, and `Release`; `RelWithAsserts` keeps optimization without defining `NDEBUG`. Caller `CMAKE_C_FLAGS`, `CMAKE_CXX_FLAGS`, and configuration-specific flags are no longer overwritten. Explicit optimization, debug-symbol, and fast-math flags suppress conflicting project defaults. Set `STAR_DEBUG_SYMBOL_LEVEL` to `default`, `minimal`, or `full`; `minimal` uses GCC `-g1` or Clang `-gline-tables-only` for smaller sanitizer/assert builds. Nix uses this option instead of patching CMake flag strings. Compiler launchers supplied by callers are preserved, except GNU+sccache, whose cached object files omit module BMIs.

C++20 modules use `.cppm` interface units; modules with implementation-heavy dependencies have companion `.cpp` files, while naturally self-contained modules remain in one file. Game Lua bindings live in `source/game/scripting/`. Import their factories and other exported APIs (for example, the HTTP trust callback functions in `star.lua_http_bindings`) instead of including the removed binding headers: after normal includes in `.cpp` files, or after `export module` in another module interface. Register game interfaces in the `star_game_module_interfaces` CMake file set and companion implementations in `star_game_modules` in `source/game/CMakeLists.txt`; consumers still link the complete static library `star_game_modules`.
Keep interfaces lean when callers include heavy game headers. Sixteen game binding modules, four frontend modules, client rendering, converted game databases and services, EmoteProcessor, world generation, TMX dungeon parts, and the split object modules separate interfaces from implementations without conditional implementation macros. Named implementation units use `module star.*;` after their global module fragment. CMake compiles companion `.cpp` files directly into the game/frontend static module libraries; separate interface OBJECT providers expose BMIs without introducing implementation dependency cycles. Client rendering's companion `.cpp` is a normal `starbound` source that skips the PCH. Enable `CXX_SCAN_FOR_MODULES` on implementation targets. This preserves the thin BMI boundary: GCC 16 rejects private module fragments and has rejected imported `MVariant` declarations and duplicate constrained JSON-vector iterator manglings when implementation-only instantiations are serialized into heavy interfaces.
`star.movement_controller_lua_bindings` follows the same interface/implementation split. `StarLuaActorMovementComponent.hpp` imports its factory for the actor callback template; targets including this header transitively must link `star_game_modules` and enable `CXX_SCAN_FOR_MODULES`, including `star_windowing`.
`star.world_lua_bindings` exports its two factories with using-declarations from the global module fragment. Its companion `.cpp` remains an ordinary translation unit: entity headers transitively import this interface through `StarLuaComponents.hpp`, so a named implementation unit would import itself before its module declaration. Keeping those dependencies out of the interface also avoids a cycle through imported object modules. Server module compilation links the game interface provider; server executables link the complete `star_game_modules` library. `update_tilesets` and `fix_embedded_tilesets` enable module scanning for their transitive game-header imports.
`star.dance_database` exports `DanceStep`, `Dance`, `DanceDatabase` and their pointer aliases through using-declarations from the global module fragment, preserving the global forward declarations in `StarHumanoid.hpp` and `StarRoot.hpp`. Its companion `.cpp` remains an ordinary translation unit importing the interface; consumers import the module instead of including the removed `StarDanceDatabase.hpp`.
`star.tech_database` follows the same global-type pattern, exporting `TechDatabase`, its pointer aliases, `TechConfig`, `TechType`, `TechTypeNames`, and the database exception types. Import it instead of the removed `StarTechDatabase.hpp`; existing global forward declarations in `StarRoot.hpp` remain valid, and its companion `.cpp` remains an ordinary translation unit.
`star.codex_database` and `star.collection_database` use the same global-type pattern with ordinary companion `.cpp` implementations. Import them instead of their removed database headers. The collection interface exports `Collection`, `Collectable`, `CollectionType`, and `CollectionTypeNames` alongside the database and exception types; it only needs the bidirectional-map header and a forward declaration of `Json`. Shared Lua converter headers forward-declare the collection types, while the converter implementation and actual database consumers import the module explicitly. Avoid putting database imports into widely included converter headers: this can spread dependencies and trigger GCC 16 imported-template linkage/mangling conflicts in otherwise unrelated modules.
`star.stagehand_database` and `star.emote_processor` preserve their global class identities and pointer aliases with the same exported using-declarations and ordinary companion `.cpp` implementations. The stagehand module also exports its exception types and the `Stagehand` forward declaration and pointer aliases. `StarStagehand.hpp` no longer includes a database header; Root, command processing, World Lua bindings, and world generation import the database at their actual call sites. The emote interface includes only `StarString.hpp` and an opaque `HumanoidEmote` declaration, keeping the full Humanoid dependency in its implementation and existing Player/Npc consumers. Include `StarHumanoid.hpp` when the emote enumerator definitions are needed.
`star.damage_database` and `star.effect_source_database` likewise preserve their global types and pointer aliases with thin interfaces and ordinary companion `.cpp` implementations. Import them at actual database call sites instead of including the removed headers. The damage interface exports `DamageKind`, `DamageEffect`, `ElementalType`, and `TargetMaterial`; its opaque `HitType` declaration keeps the full game-type dependency out of the BMI. Include `StarDamageTypes.hpp` when hit enumerators or their name map are needed. The effect-source interface exports its source/config/database types and the `particlesFromDefinition` and `soundsFromDefinition` helpers, but only forward-declares `Particle` and `AudioInstance`; include `StarParticle.hpp` or `StarMixer.hpp` when their definitions are needed. `StarEffectEmitter.hpp` only forward-declares `EffectSource`; its implementation and WorldClient explicitly import the effect-source module.
`star.particle_database`, `star.radio_message_database`, `star.status_effect_database`, `star.quest_template_database`, `star.spawn_type_database`, `star.tenant_database`, `star.statistics_database`, and `star.versioning_database` use global-fragment types, exported using-declarations, and ordinary companion `.cpp` implementations. Import these modules instead of their removed database headers; `StarRoot.hpp` retains its global forward declarations. Import at actual call sites, including inline particle creation in `StarLuaAnimationComponent.hpp`. Headers storing complete values also need the corresponding interfaces: Player/Popup for `RadioMessage`, StatusController for status-effect config, Spawner/Biome for spawn profiles, and WorldStorage for `VersionedJson`. Foundation types are not implicitly exported: include their defining headers when needed, such as `StarStatusTypes.hpp` for `UniqueStatusEffect` and `StarItemDescriptor.hpp` in quest parsing.
VersioningDatabase retains its by-value `LuaRoot` member. `StarLuaRoot.hpp` includes only its listener dependency rather than Root; callers using `Root` explicitly include `StarRoot.hpp`. WorldStorage parses the LuaRoot dependency before importing versioning, and Spawner includes WeightedPool before importing spawn types. The `make_versioned_json` and `dump_versioned_json` utilities import the versioning interface and enable CMake module scanning.
`star.universe_server_lua_bindings`, `star.micro_dungeon`, `star.loungeable_object`, and `star.physics_object` have thin interfaces with named companion implementation units. `star.world_generation` and `star.farmable_object` use global-fragment classes and ordinary companion imports to avoid imported-template conflicts in WorldServer and Lua userdata consumers. World generation includes complete facade bases but only forward-declares its server, plant, and material/liquid database dependencies; its companion imports `star.material_database` before foundation-only collision helpers, then imports the heavy world-generation interface after those helpers to avoid a GCC 16 shared-pointer instantiation ICE. Dungeon rule constructors that traverse JSON arrays live in `StarDungeonGenerator.cpp`, not its shared header. Object fields and behavior remain unchanged; the naturally small `star.teleporter_object` stays single-file.
`star.liquids_database`, `star.terrain_database`, `star.tileset_database`, `star.behavior_database`, `star.ai_database`, `star.species_database`, `star.vehicle_database`, and `star.projectile_database` likewise preserve global type identities and pointer aliases through exported using-declarations, with ordinary companion implementations. Their old database headers are removed; Root's global forwards remain unchanged. Import at actual call sites and include complete Projectile/Vehicle definitions where returned entities are dereferenced, not in database interfaces. Terrain selector interfaces import the terrain database after their module declaration. The tileset module exports Tiled properties and conversion templates; consumers still explicitly include foundation headers for Json, TilePixels, and lexicalCast.
`star.root_lua_bindings`, `star.behavior_lua_bindings`, and `star.dungeon_tmx_part` retain thin named interfaces with companion implementations. The shared Lua converter header only forward-declares BehaviorState, Blackboard, and NodeStatus; its implementation includes the complete behavior state. Avoid spreading behavior-database imports through this shared header, and preserve Json-before-Lua ordering in the behavior-state header to avoid GCC 16 SIMD linkage conflicts. `star_rendering` links the game interface provider and enables module scanning for liquid-database imports; final executables retain the complete game archive dependency.
The remaining `star.plant_database`, `star.npc_database`, `star.object_database`, `star.item_database`, `star.material_database`, `star.monster_database`, `star.celestial_database`, `star.image_metadata_database`, and `star.biome_database` follow the same thin global-type interface and ordinary companion pattern. The 32 game `*Database` file families use modules; the tree contains 124 interfaces and 76 companions overall. Their old headers are removed, while Root's global forwards, original class layouts, pointer aliases, exceptions, templates, and inline material accessors remain intact. Import databases at actual consumers, and explicitly include complete returned entity/biome types and unrelated foundation headers when needed.
Keep complete-value imports in Npc/Monster headers for their variants and in biome placement/parallax for plant variants. Monster parses `StarTtlCache.hpp` before its database import; Parallax parses `StarTileDamage.hpp` before its plant import to avoid GCC 16 imported-header redeclarations. `StarPlant.hpp` only needs variant forwards. `StarItem.hpp` includes `StarQuestDescriptor.hpp`, not the heavy quest/Lua/world dependency chain. Celestial graphics, net packets, and world templates use their lighter celestial parameter/type headers; UniverseServer forward-declares CelestialMasterDatabase. Include CollisionBlock/ItemDescriptor, AssetPath, Casting, InventoryTypes, or DataStreamDevices explicitly where their APIs are used rather than relying on removed database headers.
`star.stored_functions`, `star.treasure`, `star.name_generator`, `star.rebuilder`, `star.stat_set`, `star.stat_collection`, `star.animation`, and `star.ambient` preserve global type identities and original layouts with thin interfaces, exported using-declarations, and ordinary companion implementations. Import them instead of the removed service headers. StatCollection imports `star.stat_set` before its global definition because it stores StatSet by value. Shared headers storing Animation, AmbientManager, or StatCollection values import their complete types; pointer-only dependencies remain forward-declared. Include `StarDrawable.hpp` when consuming `Animation::drawable()` results, and include ParametricFunction/MultiTable headers when using stored-function table foundations.
Keep foundational definitions ahead of transitive service imports on GCC 16: ObjectDatabase, ItemDrop, and Player parse JsonExtra before animation imports; MovementControllerLuaBindings parses Json before Lua; ToolUser parses Object's foundations first. ErrorScreen parses InputEvent before InterfaceCursor, and the cursor implementation parses Root before its animation-containing header. `star.item_lua_bindings` now exports only its factory declaration with LuaCallbacks/Item forwards; its unchanged factory and callback bodies live in a named companion, avoiding implementation-template serialization and the observed MainInterface BMI-loading failure.
`star.player_blueprints`, `star.player_codexes`, `star.player_log`, `star.player_tech`, `star.player_universe_map`, `star.universe_settings`, `star.world_structure`, and `star.chat_processor` use the same thin global-type interfaces and ordinary companion implementations. Their old headers are removed; Player and other pointer-only consumers retain their global forwards. Export all six existing `STAR_CLASS` pointer aliases, but preserve PlayerCodexes' original single `PlayerCodexesPtr` alias rather than inventing aliases. PlayerTech imports `star.tech_database` before its global definition for by-value TechType keys; consumers naming TechType or TechTypeNames import the database directly. PlayerUniverseMap keeps complete celestial/warp/orbit foundations, exports its bookmark types and conversion templates, and retains comparison template bodies in the interface. UniverseServer imports universe settings for its by-value flag-action variant; WorldClient and WorldServer import world structure for their by-value members.
On GCC 16, CodexItem imports player codexes after its module declaration and foundation includes; CodexInterface parses Uuid before the codex import. BookmarkInterface parses Json, Warping, CelestialCoordinate, SystemWorld, and Pane before importing the universe map; TeleportDialog imports after that foundation-containing header. Keep unused state imports out of BlueprintItem, UnlockItem, AiInterface, and Inventory's header; Inventory imports player tech and the tech database in its actual implementation. The optimized ASan/UBSan standalone consumers exercised original pointer identities, real asset decoding, state transitions and serialization, chat/RPC routing, eight fresh-process malformed-asset rejections, and actual PlayerFactory disk roundtrips and player Lua callbacks; existing core and ten selected game regression cases also passed with unchanged planet and ship snapshot hashes.
`star.signal_handler` includes its Windows minidump implementation directly; the core module dependency group links `dbghelp` on Windows, alongside the core exception implementation's existing dependency.
Use persistent CMake/Ninja build directories under the matching Nix development environment for incremental compilation. Synchronize changed contents only and retain objects, PCH, and module BMIs; use separate directories for incompatible compiler/sanitizer profiles. Record the matching environment with `nix develop --profile`, then enter that recorded profile for subsequent builds; refresh it when the toolchain or dependencies change. Apply the existing Nix ImGui literal-text patch when preparing a source tree for the package-derived sanitizer environment. Run tests and actual runtime smoke against the resulting binaries; a new clean Nix package build is not required for every iteration. On a 31 GiB VPS without swap, bound the actual compiler/build unit and leave memory for SSH and the OS; reduce concurrency under memory pressure. The initial retained GCC 16 sanitizer tree completed with six compiler jobs and a 24 GiB hard limit.


<details>
<summary>template sbinit.config for dist/ after build</summary>
<br>

```json
{
  "assetDirectories" : [
    "../assets/",
    "./mods/"
  ],

  "storageDirectory" : "./",
  "logDirectory" : "./logs/"
}
```

</details>
<details>
<summary><b>Windows</b></summary>
 
* Install [vcpkg](https://github.com/microsoft/vcpkg?tab=readme-ov-file#quick-start-windows) *globally*.
  * vcpkg recommends a short directory, such as `C:\src\vcpkg` or `C:\dev\vcpkg`.
  * If you're using Visual Studio, don't forget to run `vcpkg integrate install`!
* Set the **`VCPKG_ROOT`** environment value to your vcpkg dir, so that CMake can find it.
* Install [Ninja](https://ninja-build.org/ "Ninja Build System"). Either add it to your [**`PATH`**](## "Environment Value"), or just use [Scoop](https://scoop.sh/) (`scoop install ninja`)
* Use CMake 3.28 or newer (C++20 module dependency scanning is required); check that your IDE supplies a supported compiler and CMake.
* Open the repo directory in your IDE - it should detect the CMake project.
* Build.
  * If you're using an IDE, it should detect the correct preset and allow you to build from within.
  * Otherwise, build manually by running CMake in the **source/** directory: `cmake --preset=windows-release` then `cmake --build --preset=windows-release`
* The built binaries will be in **dist/**. Copy the DLLs from **lib/windows/** and the **sbinit.config** above into **dist/** so the game can run.

</details>
<details>
<summary><b>Linux (Ubuntu)</b></summary>
 
* Use CMake 3.28 or newer, Ninja 1.11 or newer, and a compiler with C++20 module scanning support. Although CMake supports GCC 14+, this project needs GCC 16: GCC 14.4 fails compiling `StarActiveItem.cppm`, and GCC 15.3 hits a module compiler bug. Ubuntu 22.04's default GCC 11 cannot scan modules. Linux GCC CI uses Ubuntu 24.04 with GCC 16 from the [Ubuntu Toolchain test PPA](https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test), setting `CC=/usr/bin/gcc-16` and `CXX=/usr/bin/g++-16` before vcpkg and CMake configuration. The CI Clang jobs install LLVM 22.
* The pinned vcpkg registry still packages jemalloc 5.3.1, which calls a libstdc++ helper removed in GCC 16. The project's `ports/jemalloc` overlay builds upstream jemalloc 5.4.0 instead; keep the overlay when configuring through `source/vcpkg-configuration.json`.
* Linux Clang CI uses LLVM 22 with libstdc++ 12. Keep `RadioMessage`'s explicit default constructor: without it, libstdc++ recursively checks the default constructibility of its `StringMap<RadioMessage>` members while compiling `StarPlayer.cpp`.
* GCC C++20 module builds do not use a compiler cache: sccache 0.7.7 can restore an object file without its required `.gcm` module interface, causing later imports to fail.
* Install dependencies:
  * `sudo apt-get install pkg-config libxmu-dev libxi-dev libgl-dev libglu1-mesa-dev libsdl2-dev python3-jinja2 ninja-build libltdl-dev`
* Clone [vcpkg](https://github.com/microsoft/vcpkg?tab=readme-ov-file#quick-start-unix) (outside the repo!) and bootstrap it with the linked instructions.
* Set the **`VCPKG_ROOT`** environment value to your new vcpkg directory, so that CMake can find it.
  *  `export VCPKG_ROOT=/replace/with/full/path/to/your/vcpkg/directory/`
* Change to the repo's **source/** directory, then run `cmake --preset=linux-release` and `cmake --build --preset=linux-release` to build.
* The built binaries will be in **dist/**. Copy the the .so libs from **lib/linux/** and the **sbinit.config** above into **dist/** so the game can run.
  * From the root dir of the repo, you can run the assembly script which is used by the GitHub Action: `scripts/ci/linux/assemble.sh`
    * This packs the game assets and copies the built binaries, premade sbinit configs & required libs into **client/** & **server/**.
 
</details>

<details>
<summary><b>Linux (Fedora)</b></summary>

Starbound in general is built from the ground up, with its own engine written in C++ on top of some basic libraries.

* CMake 3.28+, Ninja 1.11+, and GCC 16 or a compatible LLVM Clang are required for the C++20 modules. Where Ubuntu uses APT, Fedora uses DNF as package manager.

  1. `sudo dnf upgrade --refresh` to ensure your OS is up-to-date
  2. `sudo dnf install cmake`
  3. `cmake --version` to verify

* You will need at least the same dependencies ("basic libraries") as for Ubuntu. Some packages have different names or contents between Linux builds. Namely, Fedora uses "-devel" instead of "-dev" for development packages.

  1. `sudo dnf install` [pkg-config](## "will install pkgconf-pkg-config") libXmu-devel libXi-devel [libGL-devel](## "will install mesa-libGL-devel") mesa-libGLU-devel SDL2-devel python3-jinja2 ninja-build
  2. If you find out that you need any other dependencies not listed here, try finding them via [Fedora Packages](https://packages.fedoraproject.org/) first. And, preferably, improve this instruction.

* Next you will need VCPKG.

VCPKG is another package manager/dependency resolver for C++. CMake will need it to pull the rest of dependencies automatically early in the building process. If you've worked with language-specific package managers before (for example, NPM or YUM for JavaScript), VSPKG is similar. For reference, the list of dependencies VCPKG will try to install later can be found in `source/vcpkg.json`.

  1. There are many ways to get VCPKG. Here's one: `. <(curl https://aka.ms/vcpkg-init.sh -L)`. This instruction should install VCPKG in your Linux home (user profile) directory in `.vcpkg`. Note that this dir is usually hidden by default.
  2. Next you need to set your **`VCPKG_ROOT`** environment variable to the correct path. Run `. ~/.vcpkg/vcpkg-init` to bootstrap VCPKG. You may want to check if the path is now known to the system by running `printenv VCPKG_ROOT` afterwards.
  3. Step 2 (init command) should be run in **every** new Terminal (Konsole) window **before** you begin building (environment variables set in this way do not persist between terminal sessions)

* Change to the repo's **source/** directory
* *Optional.* First step for CMake is now to run VCPKG and install the remaining dependencies as per `source/vcpkg.json`. You can run this step manually via `vcpkg install` on its own to check if it works, or *skip to the next step*.

If this step throws errors, Fedora probably still lacks some packages not listed explicitly before. Read error messages to identify these packages, find them via [Fedora Packages](https://packages.fedoraproject.org/) and install with DNF. What you need most of the time is the package itself as well as its -devel and -static subpackages.
* *Optional.* Next, we can ask CMake to assemble instructions for linux build without actually running them. The instructions generated will be stored under **build/linux-release**. To do that, run `cmake --preset=linux-release` or *skip to the next step*.
* Run `cmake --build --preset=linux-release` to build. It includes previous two steps, so if any of them throw errors, you will have problems. If that's the case, run and debug them separately as described earlier, as CMake itself can just throw `Error: could not load cache` without specifying the exact problem. In case of major changes (example: you've reinstalled VCPKG to a different location and need to regenerate path to it for CMake) purge CMake cache by deleting **source/CMakeCache.txt**.

Building will take some time, be patient ;)

<details>
<summary><b>Specific problem: If your VCPKG can't build meson for libsystemd</b></summary>
<br>

Diagnosed by 

>ERROR: Value "plain" (of type "string") for combo option "Optimization level" is not one of the choices. Possible choices are (as string): "0", "g", "1", "2", "3", "s".

error in meson building logs when building libsystemd.

Fix for VCPKG is pretty fresh (May 2024) and can be found [here](https://github.com/microsoft/vcpkg/issues/37393).

</details>

* The built binaries will be in **dist/**. Copy the the .so libs from **lib/linux/** and **sbinit.config** (see beginning of this section) into **dist/** so the game can run. Sample sbinit.config can be found in **scripts/linux/**.
* From the root dir of the repo, you can run the assembly script which is used by the GitHub Action: `scripts/ci/linux/assemble.sh`. This packs the game assets and copies the built binaries, premade sbinit configs & required libs into **client_distribution/** & **server_distribution/**.

Next you need to copy original Starbound assets at **assets/packed.pak** of the Starbound copy that you own into **assets/** of either client or server dir (depending on what you're going to run).

The game now can be run by executing **client_distribution/linux/run-client.sh** (or the corresponding server bash script) from terminal.

<details>
<summary><b>Fedora-specific problem with OSS (dsp: No such audio device)</b></summary>
<br>

Diagnosed by this error message when launching *client*:

>Couldn't initialize SDL Audio: dsp: No such audio device

The reason is outlined on [StackEx](https://stackoverflow.com/questions/9248131/failed-to-open-audio-device-dev-dsp/9248166#9248166): 

> Most new Linux distributions don't provide the OSS (open sound system) compatibility layer, because access to the OSS sound device /dev/dsp was exclusive to one program at time only.

The same answer has the solution: use `padsp` to emulate dev/dsp.

* `dnf install pulseaudio-utils` to install padsp util
* execute `padsp bash run-client.sh` instead of running sh directly. To avoid doing it every time you can edit run-client.sh, replacing 

`#!/bin/sh
cd "`dirname \"$0\"`"
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./" ./starbound "$@"`

with 

`#!/bin/sh
cd "`dirname \"$0\"`"
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./" padsp ./starbound "$@"`

</details>

</details>
<details>
<summary><b>macOS</b></summary>
 
* First, you will need to have brew installed. Check out how to install [Homebrew](https://brew.sh/)
* Install cmake using `brew install cmake`
* Install ninja using `brew install ninja`
* Install pkg config using `brew install pkg-config`
* Install an upstream LLVM Clang (16 or newer) and use its matching `clang-scan-deps`; Apple's bundled Clang is not supported by CMake's module scanner. For example, `brew install llvm@22`, then set `CC="$(brew --prefix llvm@22)/bin/clang"` and `CXX="$(brew --prefix llvm@22)/bin/clang++"` before configuring either macOS preset. Homebrew provides a prebuilt `llvm@22` bottle for macOS 15 ARM (used by CI, targeting macOS 14); on macOS 14 ARM and macOS Intel, installing this version may instead build LLVM from source for hours.
* The Intel macOS CI job downloads the [official prebuilt LLVM 20.1.7 X64 release](https://github.com/llvm/llvm-project/releases/tag/llvmorg-20.1.7) and verifies its SHA-256 instead of building `llvm@22` from source. Both CI jobs use the `clang-scan-deps` shipped with their selected Clang. When using upstream LLVM Clang on macOS, also set `export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"` before running vcpkg or CMake so Clang can find system headers in the macOS SDK.
* Next, install vcpkg by following the commands below.
 * Run `cd ~`. This is just so that everything is local to here. 
 * Run ` git clone https://github.com/microsoft/vcpkg.git `
 * Run `cd vcpkg && ./bootstrap-vcpkg.sh`
 * Lastly, run ``` export VCPKG_ROOT=~/vcpkg && export PATH=$VCPKG_ROOT:$PATH ```
 * This last command makes vcpkg added to the current terminal path. This lasts only while the terminal is active, and will have to be rerun for new terminal instances.
* Download the source code [here](https://github.com/OpenStarbound/OpenStarbound/archive/refs/heads/main.zip). This is the current code in main. Unpack the code to your downloads folder. 
* Unpack the zip, and open it up. Navigate to OpenStarbound-main/source using the terminal -> `cd ~/Downloads/OpenStarbound-main`. Then navigate to the source folder, using `cd source`.
  <details>
   <summary>If using an Arm Mac</summary>

    * While in the source folder in your terminal, run ` cmake --preset macos-arm-release `. This will get dependencies.
    * After that command has finished, run ` cmake --build --preset macos-arm-release `. Wait for this to finish, then go to Finder. Navigate to the OpenStarbound-main folder using Finder. 
    * There will be a folder called <b>dist</b>. Inside dist will be your game files, but you still need to do a few more things to run it.
    * First, in the OpenStarbound-main folder, there will be lib. Open lib, and open the osx folder. Inside is libsteam_api.dylib. Copy this file, and paste it into OpenStarbound-main/dist, so that it is in the same directory as the game files. 
    * Navigate back to OpenStarbound-main/lib/osx, and open up the folder arm64. Here, rename libdiscord_game_sdk.dylib to discord_game_sdk.dylib. The name must be that, or else the game won't be able to load. 
    * Grab the packed.pak file from your current Starbound install. It will be located in the assets folder. Copy that file into OpenStarbound-main/assets.
    * Make a new file called sbinit.config (Make sure it is .config, not .somethingelse), and copy and paste in the sbinit.config text from above, located right underneath the title Building. Place sbinit.config inside OpenStarbound-main/dist. To make a new file, open the program called TextEdit on your mac, paste in the sbinit.config text from above, and click File (located at the very top of your screen), then click Save. It will prompt you, asking where to save it. Save As: sbinit.config, Where: Navigate to OpenStarbound-main/dist. Find the file you just saved, and rename it to get rid of the wrong extension, making sure the full name and extension looks like sbinit.config.
    * You can now run the game by double clicking on the file called starbound in dist/. If it says unverified developer, open up the same folder where the game is in in the terminal. ` xattr -d com.apple.quarantine starbound `, which will get rid of the lock on the file. If that doesn't work, run ` sudo spctl --master-disable ` to allow all unverified apps. 
  </details>
  <details>
    <summary>If using an Intel Mac</summary>

     * While in the source folder in your terminal, run ` cmake --preset macos-release `. This will get dependencies.
     * After that command has finished, run ` cmake --build --preset macos-release `. Wait for this to finish, then go to Finder. Navigate to the OpenStarbound-main folder using Finder. 
     * There will be a folder called <b>dist</b>. Inside dist will be your game files, but you still need to do a few more things to run it.
     * First, in the OpenStarbound-main folder, there will be lib. Open lib, and open the osx folder. Inside is libsteam_api.dylib. Copy this file, and paste it into OpenStarbound-main/dist, so that it is in the same directory as the game files. 
     * Navigate back to OpenStarbound-main/lib/osx, and open up the folder x64. Here, rename libdiscord_game_sdk.dylib to discord_game_sdk.dylib. The name must be that, or else the game won't be able to load. 
     * Grab the packed.pak file from your current Starbound install. It will be located in the assets folder. Copy that file into OpenStarbound-main/assets.
     * Make a new file called sbinit.config (Make sure it is .config, not .somethingelse), and copy and paste in the sbinit.config text from above, located right underneath the title Building. Place sbinit.config inside OpenStarbound-main/dist. To make a new file, open the program called TextEdit on your mac, paste in the sbinit.config text from above, and click File (located at the very top of your screen), then click Save. It will prompt you, asking where to save it. Save As: sbinit.config, Where: Navigate to OpenStarbound-main/dist. Find the file you just saved, and rename it to get rid of the wrong extension, making sure the full name and extension looks like sbinit.config.
     * You can now run the game by double clicking on the file called starbound in dist/. If it says unverified developer, open up the same folder where the game is in in the terminal. ` xattr -d com.apple.quarantine starbound `, which will get rid of the lock on the file. If that doesn't work, run ` sudo spctl --master-disable ` to allow all unverified apps. 

  </details>
</details>
