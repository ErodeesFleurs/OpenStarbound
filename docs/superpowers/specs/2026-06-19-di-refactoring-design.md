# Dependency Injection Refactoring — Design Spec

Date: 2026-06-19
Status: Approved (target architecture + approach A)
Scope: Complete the in-flight DI refactoring (strict DI, no God Class decomposition)

## Current State

The codebase is mid-flight in a systematic migration from a `Root::singleton()` Service Locator pattern to constructor-based dependency injection. ~200+ commits have been made, but:

- **136 `Root::singleton()` call sites remain** in game/ non-test code
- **Game layer uses long positional parameter lists** (Player: 3×18-param ctors, UniverseServer: 21-param ctor, WorldServer: 4×17-param ctors duplicated 8× in UniverseServer.cpp)
- **Frontend layer already uses `XServices` aggregate structs** (MainInterfaceServices, TitleScreenServices, etc.) — clean pattern to replicate
- **7 I* interfaces coexist with concrete types**: IAssets, IConfiguration, IEntityFactory, IItemDatabase, ILiquidsDatabase, IMaterialDatabase, ISpeciesDatabase — mixed inconsistently
- **Repetitive null-check boilerplate**: 18 consecutive `if (!m_X) throw` blocks in Player.cpp

## Target Architecture

### Root becomes composition root (not singleton)

- `Root` retained as a class, owns 40 databases with existing lazy-load + mutex design (out of scope to change)
- Constructed once by `ClientApplication`/`ServerApplication` at startup, held as `RootPtr`
- Provides factory methods: `makeWorldServerServices()`, `makePlayerServices()`, `makeUniverseServerServices()`, etc.
- Provides reload callbacks as `function<void()>` for Services structs
- **Removed**: `Root::singleton()`, `RootBase::singletonPtr()`, `RootBase::s_singleton` atomic, `RootBase::singleton()`
- `Root` no longer enforces uniqueness via `compare_exchange`

### Remove all I* interfaces

All 7 I* interfaces deleted. Dependencies use concrete `*ConstPtr` types:
- `IAssetsConstPtr` → `AssetsConstPtr`
- `IConfigurationConstPtr` → `ConfigurationConstPtr`
- `IEntityFactoryConstPtr` → `EntityFactoryConstPtr`
- `IItemDatabaseConstPtr` → `ItemDatabaseConstPtr`
- `ILiquidsDatabaseConstPtr` → `LiquidsDatabaseConstPtr`
- `IMaterialDatabaseConstPtr` → `MaterialDatabaseConstPtr`
- `ISpeciesDatabaseConstPtr` → `SpeciesDatabaseConstPtr`

Concrete classes drop `: public IAssets` (etc.) inheritance.
`World` interface return types change from `IAssetsConstPtr`/`IItemDatabaseConstPtr` to `AssetsConstPtr`/`ItemDatabaseConstPtr`.
Test stub (`StarWorldStub.hpp`) updated to return concrete types (still empty ptrs).

### Services aggregate structs for game layer

Each long-parameter-list class gets a `XServices` struct:

```cpp
struct WorldServerServices {
  AssetsConstPtr assets;
  ConfigurationConstPtr configuration;
  MaterialDatabaseConstPtr materialDatabase;
  ItemDatabaseConstPtr itemDatabase;
  ObjectDatabaseConstPtr objectDatabase;
  // ... all 17 dependencies
  function<void(ListenerWeakPtr)> registerReloadListener;
  function<void()> reloadRoot;
};
class WorldServer {
public:
  WorldServer(WorldTemplatePtr const&, IODevicePtr storage, WorldServerServices services);
};
```

Benefits:
- Constructor parameter count drops from 17→3
- 8 duplicate WorldServer construction sites in UniverseServer.cpp become single `makeWorldServerServices()` call
- Named struct fields eliminate positional-argument-swap bugs

### Validation helper

A `requireNotNull` helper collapses repetitive null checks:

```cpp
template <typename T>
void requireNotNull(SharedPtr<T> const& ptr, StringView context, StringView serviceName) {
  if (!ptr)
    throw StarException(strf("{} requires {} service", context, serviceName));
}
```

Usage: `requireNotNull(services.assets, "WorldServer", "assets");` — one line per service instead of 3.

### Lua bindings: closure-captured Services

Lua binding registration functions accept `Services const&` and capture in closures:
```cpp
void registerWorldCallbacks(LuaEngine& engine, WorldServerServices const& services) {
  // Instead of Root::singleton().materialDatabase() inside each Lua C function:
  engine.registerFunction("materialName", [materialDb = services.materialDatabase](...) { ... });
}
```

Existing `WorldServer`/`UniverseServer` already have Services after Phase 3, so the closures capture from there.

## Migration Plan (Approach A — dependency bottom-up)

### Phase 0: Infrastructure (this PR)

1. Add `requireNotNull` helper to `StarAlgorithm.hpp` or a new small header
2. Remove all 7 I* interfaces:
   - Delete `StarIAssets.hpp`, `StarIConfiguration.hpp`, `StarIEntityFactory.hpp`, `StarIItemDatabase.hpp`, `StarILiquidsDatabase.hpp`, `StarIMaterialDatabase.hpp`, `StarISpeciesDatabase.hpp`
   - Remove `: public IAssets` (etc.) from concrete classes
   - Replace all `I*ConstPtr`/`I*Ptr` usages with concrete `*ConstPtr`/`*Ptr`
   - Update `World` interface return types
   - Update `StarWorldStub.hpp` test stub
   - Update `RootBase` if it references I* types
3. Verify build + tests pass

### Phase 1: Leaf subsystems (per-PR)

Migrate `Root::singleton()` calls in leaf files (no further dependents):
- `StarDrawable.cpp` (6 calls)
- `StarAnimation.cpp` (1 call)
- `StarEntitySplash.cpp` (1 call)
- `StarEffectEmitter.cpp` (2 calls)
- `StarHumanoid.cpp` (11 calls)
- `StarTechController.cpp` (1 call)
- `StarStatusController.cpp` (3 calls)

Each file: add needed databases as constructor/member params, update callers.

### Phase 2: Mid-layer subsystems (per-PR)

- `items/` (MaterialItem, Tools, ThrownItem, InspectionTool — ~12 calls)
- `objects/` (FarmableObject, ContainerObject, Object — ~6 calls)
- celestial (CelestialParameters, CelestialGraphics, CelestialDatabase, CelestialLuaBindings — ~8 calls)
- `StarWorldTemplate.cpp`, `StarWorldLayout.cpp`, `StarWorldParameters.cpp` (~15 calls)
- `StarBiomePlacement.cpp`, `StarDungeonGenerator.cpp`, `StarDungeonTMXPart.cpp`, `StarTileDrawer.cpp`
- `StarInput.cpp`, `StarCommandProcessor.cpp`, `StarEntityFactory.cpp`, `StarVersioningDatabase.cpp`

### Phase 3: Top-level Services structs (per-PR)

- Introduce `WorldServerServices`, `WorldClientServices`, `UniverseServerServices`, `PlayerServices` structs
- Refactor constructors to accept Services
- Add `Root::makeWorldServerServices()` etc. factory methods
- Replace 8 duplicate WorldServer construction sites
- Collapse null-check boilerplate with `requireNotNull`

### Phase 4: Lua bindings (per-PR)

- `StarWorldLuaBindings.cpp` (11 singleton calls)
- `StarRootLuaBindings.cpp`, `StarBehaviorLuaBindings.cpp`, `StarCameraLuaBindings.cpp`, `StarLuaRoot.cpp`, `StarLuaComponents.cpp`, `StarLuaHttpBindings.cpp`, `StarCelestialLuaBindings.cpp`, `StarLuaAnimationComponent.hpp`
- Convert to closure-captured Services

### Phase 5: Root → composition root (final PR)

- Remove `Root::singleton()`, `RootBase::singletonPtr()`, `RootBase::s_singleton`
- Update `ClientApplication`/`ServerApplication` to hold `RootPtr` and thread it
- Update tests to construct `Root` explicitly (StarTestUniverse already does `Root::singleton()` — change to receive RootPtr)

## Verification

- Each PR: `nix run .#build-clang` (build) + `nix run .#test-clang` (core_tests, NoAssets label)
- `game_tests` needs `assets/packed.pak`; run manually if available
- No new unit tests added (per user decision — rely on existing test suite)

## Progress

### Phase 0: Infrastructure (DONE)
- Deleted 7 I* interface headers, removed inheritance, replaced all I* type aliases
- Added `requireNotNull()` helper to StarAlgorithm.hpp
- 241 files changed. Build + all tests pass.

### Phase 1: Leaf subsystems (DONE)
- Added 12 database accessors to World interface: assets, itemDatabase, objectDatabase,
  materialDatabase, liquidsDatabase, effectSourceDatabase, particleDatabase, techDatabase,
  statusEffectDatabase, plantDatabase, treasureDatabase, imageMetadataDatabase
- WorldServer/WorldClient implement new accessors (Root fallback for non-member databases)
- Migrated: Animation, EntitySplash, EffectEmitter, TechController, StatusController, Particle
- Drawable: added optional ImageMetadataDatabaseConstPtr parameter to methods (Root fallback)
- Humanoid: deferred to Phase 3 (no world() access, 11 calls)

### Phase 2: Mid-layer subsystems (DONE - exhausted easy migrations)
- Migrated items/: MaterialItem, Tools, InspectionTool (constructor keeps Root fallback)
- Migrated objects/: FarmableObject, Object, ContainerObject
- Migrated entities: Npc/Monster/Projectile (World-interface databases only)
- Migrated Plant/PlantDrop (with resolveImageMetadata helper for constructor calls)
- Migrated EffectSourceDatabase::particlesFromDefinition (optional ParticleDatabase param)
- Remaining 103 calls require structural changes (Phase 3/4/5)

### Remaining calls breakdown (103 total)
- 48: Phase 3 (Services structs - classes without world() access: Humanoid, WorldLayout, Monster, Npc, etc.)
- 21: Phase 4 (Lua bindings - closure-captured Services)
- 21: Phase 5 (Root composition root - versioningDatabase, configuration, assets fallbacks, WorldServer/Client Root fallbacks)
- 6: Permanent fallbacks (Drawable/StatusController/Plant resolveXxx helpers)
- 7: Other (Input, CelestialGraphics, etc.)

## Out of Scope

- God Class decomposition (Player/WorldServer/UniverseServer stay as-is)
- Component/ESC architecture
- Reducing Root's 42 mutexes
- Standard library migration (Maybe→std::optional, etc.)
- Mechanical modernization (enum class, constexpr, concepts)
