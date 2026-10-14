module;
import std;
import star.maybe;
import star.format;
import star.json;
import star.id_map;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.ttl_cache;
import star.game_types;
import star.material_types;
import star.data_stream;
import star.bi_map;
import star.color;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.vector;
#include "StarStrongTypedef.hpp"
import star.string;
import star.poly;
#include "StarCasting.hpp"

import star.item_descriptor;
import star.animation;
import star.particle;
import star.tile_damage;
import star.damage_types;
import star.status_types;
import star.drawable;
import star.entity_rendering_types;
import star.world_geometry;
import star.damage;
import star.entity;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;

import star.light_source;

import star.net_compatibility;
import star.byte_array;
namespace Star {

class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class Image;
using ImagePtr = std::shared_ptr<Image>;
using ImageConstPtr = std::shared_ptr<const Image>;
using ImageWeakPtr = std::weak_ptr<Image>;
using ImageConstWeakPtr = std::weak_ptr<const Image>;
using ImageUPtr = std::unique_ptr<Image>;
using ImageConstUPtr = std::unique_ptr<const Image>;
class ItemDatabase;
using ItemDatabasePtr = std::shared_ptr<ItemDatabase>;
using ItemDatabaseConstPtr = std::shared_ptr<const ItemDatabase>;
using ItemDatabaseWeakPtr = std::weak_ptr<ItemDatabase>;
using ItemDatabaseConstWeakPtr = std::weak_ptr<const ItemDatabase>;
using ItemDatabaseUPtr = std::unique_ptr<ItemDatabase>;
using ItemDatabaseConstUPtr = std::unique_ptr<const ItemDatabase>;
class RecipeDatabase;
using RecipeDatabasePtr = std::shared_ptr<RecipeDatabase>;
using RecipeDatabaseConstPtr = std::shared_ptr<const RecipeDatabase>;
using RecipeDatabaseWeakPtr = std::weak_ptr<RecipeDatabase>;
using RecipeDatabaseConstWeakPtr = std::weak_ptr<const RecipeDatabase>;
using RecipeDatabaseUPtr = std::unique_ptr<RecipeDatabase>;
using RecipeDatabaseConstUPtr = std::unique_ptr<const RecipeDatabase>;
class Object;
using ObjectPtr = std::shared_ptr<Object>;
using ObjectConstPtr = std::shared_ptr<const Object>;
using ObjectWeakPtr = std::weak_ptr<Object>;
using ObjectConstWeakPtr = std::weak_ptr<const Object>;
using ObjectUPtr = std::unique_ptr<Object>;
using ObjectConstUPtr = std::unique_ptr<const Object>;
struct ObjectOrientation;
using ObjectOrientationPtr = std::shared_ptr<ObjectOrientation>;
using ObjectOrientationConstPtr = std::shared_ptr<const ObjectOrientation>;
using ObjectOrientationWeakPtr = std::weak_ptr<ObjectOrientation>;
using ObjectOrientationConstWeakPtr = std::weak_ptr<const ObjectOrientation>;
using ObjectOrientationUPtr = std::unique_ptr<ObjectOrientation>;
using ObjectOrientationConstUPtr = std::unique_ptr<const ObjectOrientation>;
struct ObjectConfig;
using ObjectConfigPtr = std::shared_ptr<ObjectConfig>;
using ObjectConfigConstPtr = std::shared_ptr<const ObjectConfig>;
using ObjectConfigWeakPtr = std::weak_ptr<ObjectConfig>;
using ObjectConfigConstWeakPtr = std::weak_ptr<const ObjectConfig>;
using ObjectConfigUPtr = std::unique_ptr<ObjectConfig>;
using ObjectConfigConstUPtr = std::unique_ptr<const ObjectConfig>;
class ObjectDatabase;
using ObjectDatabasePtr = std::shared_ptr<ObjectDatabase>;
using ObjectDatabaseConstPtr = std::shared_ptr<const ObjectDatabase>;
using ObjectDatabaseWeakPtr = std::weak_ptr<ObjectDatabase>;
using ObjectDatabaseConstWeakPtr = std::weak_ptr<const ObjectDatabase>;
using ObjectDatabaseUPtr = std::unique_ptr<ObjectDatabase>;
using ObjectDatabaseConstUPtr = std::unique_ptr<const ObjectDatabase>;
class LuaRoot;
using LuaRootPtr = std::shared_ptr<LuaRoot>;
using LuaRootConstPtr = std::shared_ptr<const LuaRoot>;
using LuaRootWeakPtr = std::weak_ptr<LuaRoot>;
using LuaRootConstWeakPtr = std::weak_ptr<const LuaRoot>;
using LuaRootUPtr = std::unique_ptr<LuaRoot>;
using LuaRootConstUPtr = std::unique_ptr<const LuaRoot>;
class Rebuilder;
using RebuilderPtr = std::shared_ptr<Rebuilder>;
using RebuilderConstPtr = std::shared_ptr<const Rebuilder>;
using RebuilderWeakPtr = std::weak_ptr<Rebuilder>;
using RebuilderConstWeakPtr = std::weak_ptr<const Rebuilder>;
using RebuilderUPtr = std::unique_ptr<Rebuilder>;
using RebuilderConstUPtr = std::unique_ptr<const Rebuilder>;

struct ObjectExceptionTag {
  static constexpr char const* name() { return "ObjectException"; }
};
using ObjectException = StarError<ObjectExceptionTag, StarException>;

struct ObjectOrientation {
  struct Anchor {
    TileLayer layer;
    Vec2I position;
    bool tilled;
    bool soil;
    Maybe<MaterialId> material;
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
  Maybe<RectF> metaBoundBox;

  // Anchors of the object to place it in the world
  // For background tiles set in order for the object to
  // remain placed.  Must be within 1 space of the bounding box of spaces.
  // For foreground tiles this cannot logically contain any position
  // also in spaces, as objects cannot overlap with foreground tiles.
  List<Anchor> anchors;

  // if true, only one anchor needs to be valid for the orientation to be valid,
  // otherwise all anchors must be valid
  bool anchorAny;

  Maybe<Direction> directionAffinity;

  // Optional list of material spaces
  List<MaterialSpace> materialSpaces;

  // optionally override the default spaces used for interaction
  Maybe<List<Vec2I>> interactiveSpaces;

  Vec2F lightPosition;
  float beamAngle;

  List<ParticleEmissionEntry> particleEmitters;

  Maybe<PolyF> statusEffectArea;
  Json touchDamageConfig;

  static ParticleEmissionEntry parseParticleEmitter(String const& path, Json const& config);
  bool placementValid(World const* world, Vec2I const& position) const;
  bool anchorsValid(World const* world, Vec2I const& position) const;
};

// TODO: This is used very strangely and inconsistently. We go to all the trouble of populating
// this ObjectConfig structure from the JSON, but then keep around the JSON anyway. In some
// places we access the objectConfig, but in many more we use the object's configValue method
// to access the raw config JSON which means it's inconsistent which parameters can be overridden
// by instance values at various levels. This whole system needs reevaluation.
struct ObjectConfig {
  // Returns the index of the best valid orientation.  If no orientations are
  // valid, returns NPos
  std::size_t findValidOrientation(World const* world, Vec2I const& position, Maybe<Direction> directionAffinity = Maybe<Direction>()) const;

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
  Maybe<PeriodicFunction<float>> lightFlickering;

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

  Maybe<float> minimumLiquidLevel;
  Maybe<float> maximumLiquidLevel;
  float liquidCheckInterval;

  float health;

  Json animationConfig;

  List<ObjectOrientationPtr> orientations;

  // If true, the object will root - it will prevent the blocks it is
  // anchored to from being destroyed directly, and damage from those
  // blocks will be redirected to the object
  bool rooting;

  bool biomePlaced;
};

class ObjectDatabase {
public:
  static List<Vec2I> scanImageSpaces(ImageConstPtr const& image, Vec2F const& position, float fillLimit, bool flip = false);
  static Json parseTouchDamage(String const& path, Json const& touchDamage);
  static List<ObjectOrientationPtr> parseOrientations(String const& path, Json const& configList, Json const& baseConfig);

  ObjectDatabase();

  void cleanup();

  StringList allObjects() const;
  bool isObject(String const& name) const;

  ObjectConfigPtr getConfig(String const& objectName) const;
  List<ObjectOrientationPtr> const& getOrientations(String const& objectName) const;

  ObjectPtr createObject(String const& objectName, Json const& objectParameters = JsonObject()) const;
  ObjectPtr diskLoadObject(Json const& diskStore) const;
  ObjectPtr netLoadObject(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

  bool canPlaceObject(World const* world, Vec2I const& position, String const& objectName) const;
  // If the object is placeable in the given position, creates the given object
  // and sets its position and direction and returns it, otherwise returns
  // null.
  ObjectPtr createForPlacement(World const* world, String const& objectName, Vec2I const& position,
      Direction direction, Json const& parameters = JsonObject()) const;

  List<Drawable> cursorHintDrawables(World const* world, String const& objectName, Vec2I const& position,
      Direction direction, Json parameters = {}) const;

private:
  static ObjectConfigPtr readConfig(String const& path);

  StringMap<String> m_paths;
  mutable Mutex m_cacheMutex;
  mutable HashTtlCache<String, ObjectConfigPtr> m_configCache;

  RebuilderPtr m_rebuilder;
};

}

export module star.object_database;

export namespace Star {
  using ::Star::ObjectExceptionTag;
  using ::Star::ObjectException;
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::Image;
  using ::Star::ImagePtr;
  using ::Star::ImageConstPtr;
  using ::Star::ImageWeakPtr;
  using ::Star::ImageConstWeakPtr;
  using ::Star::ImageUPtr;
  using ::Star::ImageConstUPtr;
  using ::Star::ItemDatabase;
  using ::Star::ItemDatabasePtr;
  using ::Star::ItemDatabaseConstPtr;
  using ::Star::ItemDatabaseWeakPtr;
  using ::Star::ItemDatabaseConstWeakPtr;
  using ::Star::ItemDatabaseUPtr;
  using ::Star::ItemDatabaseConstUPtr;
  using ::Star::RecipeDatabase;
  using ::Star::RecipeDatabasePtr;
  using ::Star::RecipeDatabaseConstPtr;
  using ::Star::RecipeDatabaseWeakPtr;
  using ::Star::RecipeDatabaseConstWeakPtr;
  using ::Star::RecipeDatabaseUPtr;
  using ::Star::RecipeDatabaseConstUPtr;
  using ::Star::Object;
  using ::Star::ObjectPtr;
  using ::Star::ObjectConstPtr;
  using ::Star::ObjectWeakPtr;
  using ::Star::ObjectConstWeakPtr;
  using ::Star::ObjectUPtr;
  using ::Star::ObjectConstUPtr;
  using ::Star::ObjectOrientation;
  using ::Star::ObjectOrientationPtr;
  using ::Star::ObjectOrientationConstPtr;
  using ::Star::ObjectOrientationWeakPtr;
  using ::Star::ObjectOrientationConstWeakPtr;
  using ::Star::ObjectOrientationUPtr;
  using ::Star::ObjectOrientationConstUPtr;
  using ::Star::ObjectConfig;
  using ::Star::ObjectConfigPtr;
  using ::Star::ObjectConfigConstPtr;
  using ::Star::ObjectConfigWeakPtr;
  using ::Star::ObjectConfigConstWeakPtr;
  using ::Star::ObjectConfigUPtr;
  using ::Star::ObjectConfigConstUPtr;
  using ::Star::ObjectDatabase;
  using ::Star::ObjectDatabasePtr;
  using ::Star::ObjectDatabaseConstPtr;
  using ::Star::ObjectDatabaseWeakPtr;
  using ::Star::ObjectDatabaseConstWeakPtr;
  using ::Star::ObjectDatabaseUPtr;
  using ::Star::ObjectDatabaseConstUPtr;
  using ::Star::LuaRoot;
  using ::Star::LuaRootPtr;
  using ::Star::LuaRootConstPtr;
  using ::Star::LuaRootWeakPtr;
  using ::Star::LuaRootConstWeakPtr;
  using ::Star::LuaRootUPtr;
  using ::Star::LuaRootConstUPtr;
  using ::Star::Rebuilder;
  using ::Star::RebuilderPtr;
  using ::Star::RebuilderConstPtr;
  using ::Star::RebuilderWeakPtr;
  using ::Star::RebuilderConstWeakPtr;
  using ::Star::RebuilderUPtr;
  using ::Star::RebuilderConstUPtr;
}
