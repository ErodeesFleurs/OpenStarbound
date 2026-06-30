#pragma once

#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarAnimation.hpp"
#include "StarAssetPath.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

struct Particle {
  enum class Type {
    // Variance is basically a null type, used only for varying other particles
    // by amounts.
    Variance,

    Ember,
    Textured,
    Animated,
    Streak,
    Text
  };
  static EnumMap<Type> const TypeNames;

  enum class DestructionAction {
    None,
    Image,
    Fade,
    Shrink
  };
  static EnumMap<DestructionAction> const DestructionActionNames;

  enum class Layer {
    Back,
    Middle,
    Front
  };
  static EnumMap<Layer> const LayerNames;

  Particle() = default;
  // If particle is type Textured, then the image name is considered relative
  // to the given asset path
  explicit Particle(Json const& config, String const& assetsPath = "/", AssetsConstPtr assets = {}, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {});

  Json toJson() const;

  void translate(Vec2F const& pos);

  // Updates position, velocity, rotation, and timeToLive.
  void update(float dt, Vec2F const& wind = Vec2F());

  bool dead() const;

  // Apply random variance to this particle based on a "variance" particle that
  // contains the maximum amount of variance for each field.
  void applyVariance(Particle const& variance);

  // Stops particle and sets time to live to 0.0 (triggering destruction)
  void collide(Vec2F const& collisionPosition);
  // Immediately triggers destruction of particle with / without destruction
  // action
  void destroy(bool withDestruction);

  // Internally called by update() / collide() / destruct()
  void destructionUpdate();

  void initializeAnimation(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  Type type = Type::Variance;

  // Defaults to 1.0, 1.0 will produce a reasonable size particle for whatever
  // the type is.
  float size = 0.0f;
  float baseSize = 0.0f; // track the original size for shrink destruction action

  // Used differently depending on the type of the particle.
  String string;
  AssetPath image;
  DirectivesGroup directives;

  Color color = Color::White;
  Color light = Color::Clear;
  float fade = 0.0f;
  bool fullbright = false;

  Vec2F position;
  Vec2F velocity;
  Vec2F finalVelocity;
  Vec2F approach;

  bool flippable = true;
  bool flip = false;

  float rotation = 0.0f;
  float angularVelocity = 0.0f;

  float length = 0.0f;

  DestructionAction destructionAction = DestructionAction::None;
  AssetPath destructionImage;
  float destructionTime = 0.0f;
  bool destructionSet = false;

  float timeToLive = 0.0f;
  Layer layer = Layer::Middle;

  bool collidesForeground = true;
  bool collidesLiquid = true;
  bool underwaterOnly = false;

  bool ignoreWind = true;

  bool trail = false;

  Maybe<Animation> animation;

  // Non-serialized: assets used for lazy animation initialization in update()
  AssetsConstPtr assets;
  ImageMetadataDatabaseConstPtr imageMetadataDatabase;
};

DataStream& operator<<(DataStream& ds, Particle const& particle);
DataStream& operator>>(DataStream& ds, Particle& particle);

using ParticleVariantCreator = function<Particle()>;
ParticleVariantCreator makeParticleVariantCreator(Particle particle, Particle variance);

}
