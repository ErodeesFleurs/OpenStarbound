module;
import std;
import star.string;
import star.json;
import star.id_map;
import star.maybe;
import star.variant;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.set;


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.physics_entity;
import star.movement_controller;
import star.mobile_entity;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;

import star.net_element_basic_fields;
import star.net_element_system;

import star.net_compatibility;
import star.byte_array;
namespace Star {

class RenderCallback;

class PlantDrop : public virtual MobileEntity {
public:
  PlantDrop(List<Plant::PlantPiece> pieces, Vec2F const& position, Vec2F const& strikeVector, String const& description,
      bool upsideDown, Json stemConfig, Json foliageConfig, Json saplingConfig,
      bool master, float random);
  PlantDrop(ByteArray const& netStore, NetCompatibilityRules rules = {});

  ByteArray netStore(NetCompatibilityRules rules = {});

  EntityType entityType() const override;

  void init(World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;

  String description() const override;

  std::pair<ByteArray, std::uint64_t> writeNetState(std::uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;

  void enableInterpolation(float extrapolationHint = 0.0f) override;
  void disableInterpolation() override;

  Vec2F position() const override;
  RectF metaBoundBox() const override;

  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;

  void setPosition();
  void setVelocity(Vec2F const& position);

  RectF collisionRect() const;

  void update(float dt, std::uint64_t currentStep) override;

  void render(RenderCallback* renderCallback) override;
  
  MovementController* movementController() override;

private:
  struct PlantDropPiece {
    PlantDropPiece();
    AssetPath image;
    Vec2F offset;
    int segmentIdx;
    Plant::PlantPieceKind kind;
    bool flip;
  };

  void particleForPlantPart(PlantDropPiece const& piece, String const& mode, Json const& mainConfig, RenderCallback* renderCallback);

  NetElementTopGroup m_netGroup;
  String m_description;
  float m_time;
  MovementController m_movementController;
  RectF m_collisionRect;
  RectF m_boundingBox;
  float m_rotationRate;
  float m_rotationFallThreshold;
  float m_rotationCap;
  List<PlantDropPiece> m_pieces;
  Json m_stemConfig;
  Json m_foliageConfig;
  Json m_saplingConfig;
  bool m_master;
  bool m_firstTick;
  NetElementBool m_spawnedDrops;
  bool m_spawnedDropEffects;
};

}

export module star.plant_drop;

export namespace Star {
  using ::Star::RenderCallback;
  using ::Star::PlantDrop;
}
