#pragma once

#include "StarConfig.hpp"
#include "StarDamageTypes.hpp"
#include "StarEntityRenderingTypes.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarParticle.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarStatusTypes.hpp"
#include "StarTileDamage.hpp"
#include "StarTileEntity.hpp"
#include "StarTtlCache.hpp"

import std;

namespace Star {

class Object;
class Rebuilder;

using ObjectException = ExceptionDerived<"ObjectException">;

struct ObjectOrientation {
  struct Anchor {
    TileLayer layer;
    Vec2I position;
    bool tilled;
    bool soil;
    std::optional<MaterialId> material;
  };

  struct ParticleEmissionEntry {
    float particleEmissionRate;
    float particleEmissionRateVariance;
    // Particle positions are considered relative to image pixels, and are
    // flipped with image flipping
    Particle particle;
    Particle particleVariance;
    bool placeInSpaces;
  };

  // The JSON values that were used to configure this orientation.
  Json config;

  EntityRenderLayer renderLayer;
  List<Drawable> imageLayers;
  bool flipImages;

  // Offset of image from (0, 0) object position, in tile coordinates
  Vec2F imagePosition;

  // If an object has frames > 1, then the image name will have the marker
  // "{frame}" replaced with an integer in [0, frames)
  unsigned frames;
  float animationCycle;

  // Spaces the object occupies.  By default, this is simply the single space
  // at the object position, but can be specified in config as either a list of
  // Vec2I, or by setting a threshold value using "spaceScanning", which will
  // scan the image (frame 1) for non-transparent pixels.
  List<Vec2I> spaces;
  RectI boundBox;

  // Allow an orientation to override the metaboundbox in case you don't want to
  // specify spaces
  std::optional<RectF> metaBoundBox;

  // Anchors of the object to place it in the world
  // For background tiles set in order for the object to
  // remain placed.  Must be within 1 space of the bounding box of spaces.
  // For foreground tiles this cannot logically contain any position
  // also in spaces, as objects cannot overlap with foreground tiles.
  List<Anchor> anchors;

  // if true, only one anchor needs to be valid for the orientation to be valid,
  // otherwise all anchors must be valid
  bool anchorAny;

  std::optional<Direction> directionAffinity;

  // Optional list of material spaces
  List<MaterialSpace> materialSpaces;

  // optionally override the default spaces used for interaction
  std::optional<List<Vec2I>> interactiveSpaces;

  Vec2F lightPosition;
  float beamAngle;

  List<ParticleEmissionEntry> particleEmitters;

  std::optional<PolyF> statusEffectArea;
  Json touchDamageConfig;

  static auto parseParticleEmitter(String const& path, Json const& config) -> ParticleEmissionEntry;
  auto placementValid(World const* world, Vec2I const& position) const -> bool;
  auto anchorsValid(World const* world, Vec2I const& position) const -> bool;
};

// TODO: This is used very strangely and inconsistently. We go to all the trouble of populating
// this ObjectConfig structure from the JSON, but then keep around the JSON anyway. In some
// places we access the objectConfig, but in many more we use the object's configValue method
// to access the raw config JSON which means it's inconsistent which parameters can be overridden
// by instance values at various levels. This whole system needs reevaluation.
struct ObjectConfig {
  // Returns the index of the best valid orientation.  If no orientations are
  // valid, returns std::numeric_limits<std::size_t>::max()
  auto findValidOrientation(World const* world, Vec2I const& position, std::optional<Direction> directionAffinity = std::nullopt) const -> size_t;

  String path;
  // The JSON values that were used to configure this Object
  Json config;

  String name;
  String type;
  String race;
  String category;
  StringList colonyTags;
  StringList scripts;
  StringList animationScripts;

  unsigned price;
  bool printable;
  bool scannable;

  bool interactive;

  StringMap<Color> lightColors;
  LightType lightType;
  float pointBeam;
  float beamAmbience;
  std::optional<PeriodicFunction<float>> lightFlickering;

  String soundEffect;
  float soundEffectRangeMultiplier;

  List<PersistentStatusEffect> statusEffects;
  Json touchDamageConfig;

  bool hasObjectItem;
  bool retainObjectParametersInItem;

  bool smashable;
  bool smashOnBreak;
  bool unbreakable;
  String smashDropPool;
  List<List<ItemDescriptor>> smashDropOptions;
  StringList smashSoundOptions;
  JsonArray smashParticles;

  String breakDropPool;
  List<List<ItemDescriptor>> breakDropOptions;

  TileDamageParameters tileDamageParameters;
  float damageShakeMagnitude;
  String damageMaterialKind;

  EntityDamageTeam damageTeam;

  std::optional<float> minimumLiquidLevel;
  std::optional<float> maximumLiquidLevel;
  float liquidCheckInterval;

  float health;

  Json animationConfig;

  List<Ptr<ObjectOrientation>> orientations;

  // If true, the object will root - it will prevent the blocks it is
  // anchored to from being destroyed directly, and damage from those
  // blocks will be redirected to the object
  bool rooting;

  bool biomePlaced;
};

class ObjectDatabase {
public:
  static auto scanImageSpaces(ConstPtr<Image> const& image, Vec2F const& position, float fillLimit, bool flip = false) -> List<Vec2I>;
  static auto parseTouchDamage(String const& path, Json const& touchDamage) -> Json;
  static auto parseOrientations(String const& path, Json const& configList, Json const& baseConfig) -> List<Ptr<ObjectOrientation>>;

  ObjectDatabase();

  void cleanup();

  auto allObjects() const -> StringList;
  auto isObject(String const& name) const -> bool;

  auto getConfig(String const& objectName) const -> Ptr<ObjectConfig>;
  auto getOrientations(String const& objectName) const -> List<Ptr<ObjectOrientation>> const&;

  auto createObject(String const& objectName, Json const& objectParameters = JsonObject()) const -> Ptr<Object>;
  auto diskLoadObject(Json const& diskStore) const -> Ptr<Object>;
  auto netLoadObject(ByteArray const& netStore, NetCompatibilityRules rules = {}) const -> Ptr<Object>;

  auto canPlaceObject(World const* world, Vec2I const& position, String const& objectName) const -> bool;
  // If the object is placeable in the given position, creates the given object
  // and sets its position and direction and returns it, otherwise returns
  // null.
  auto createForPlacement(World const* world, String const& objectName, Vec2I const& position,
                          Direction direction, Json const& parameters = JsonObject()) const -> Ptr<Object>;

  auto cursorHintDrawables(World const* world, String const& objectName, Vec2I const& position,
                           Direction direction, Json parameters = {}) const -> List<Drawable>;

private:
  static auto readConfig(String const& path) -> Ptr<ObjectConfig>;

  StringMap<String> m_paths;
  mutable Mutex m_cacheMutex;
  mutable HashTtlCache<String, Ptr<ObjectConfig>> m_configCache;

  Ptr<Rebuilder> m_rebuilder;
};

}// namespace Star
