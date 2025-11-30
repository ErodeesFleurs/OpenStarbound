#include "StarPlantDropAdapter.hpp"
#include "StarRoot.hpp"
#include "StarAssets.hpp"
#include "StarRandom.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarWorld.hpp"
#include "StarItemDrop.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarParticleDatabase.hpp"
#include "StarEntityRendering.hpp"
#include "StarMathCommon.hpp"

namespace Star {
namespace ECS {

// Static factory methods

shared_ptr<PlantDropAdapter> PlantDropAdapter::create(
    World* ecsWorld,
    List<Plant::PlantPiece> pieces,
    Vec2F const& position,
    Vec2F const& strikeVector,
    String const& description,
    bool upsideDown,
    Json stemConfig,
    Json foliageConfig,
    Json saplingConfig,
    bool master,
    float random) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantDropAdapter>(ecsWorld, entity);
  adapter->setupComponents(pieces, position, strikeVector, description, upsideDown,
    stemConfig, foliageConfig, saplingConfig, master, random);
  return adapter;
}

shared_ptr<PlantDropAdapter> PlantDropAdapter::createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantDropAdapter>(ecsWorld, entity);
  
  // Add tag
  adapter->addComponent<PlantDropTag>();
  
  // Parse network data
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);
  
  auto& dropData = adapter->addComponent<PlantDropDataComponent>();
  ds >> dropData.time;
  ds >> dropData.master;
  ds >> dropData.description;
  ds >> dropData.boundingBox;
  ds >> dropData.collisionRect;
  ds >> dropData.rotationRate;
  
  ds.readContainer(dropData.pieces,
    [](DataStream& ds, PlantDropPiece& piece) {
      ds.read(piece.image);
      ds.read(piece.offset[0]);
      ds.read(piece.offset[1]);
      ds.read(piece.flip);
      ds.read(piece.kind);
    });
  
  ds >> dropData.stemConfig;
  ds >> dropData.foliageConfig;
  ds >> dropData.saplingConfig;
  
  dropData.firstTick = true;
  dropData.spawnedDropEffects = true;
  
  // Setup other components
  auto& entityType = adapter->addComponent<EntityTypeComponent>();
  entityType.type = EntityType::PlantDrop;
  entityType.ephemeral = true;
  
  auto& transform = adapter->addComponent<TransformComponent>();
  transform.position = Vec2F();
  
  auto& velocity = adapter->addComponent<VelocityComponent>();
  velocity.velocity = Vec2F();
  
  auto& bounds = adapter->addComponent<BoundsComponent>();
  bounds.metaBoundBox = dropData.boundingBox;
  bounds.collisionArea = dropData.collisionRect;
  
  auto& physics = adapter->addComponent<PhysicsBodyComponent>();
  physics.mass = 1.0f;
  physics.gravityMultiplier = 0.2f;
  physics.collisionEnabled = true;
  physics.gravityEnabled = true;
  
  // Network sync
  auto& netSync = adapter->addComponent<NetworkSyncComponent>();
  netSync.netVersion = 1;
  
  adapter->addComponent<InterpolationComponent>();
  
  return adapter;
}

// Constructor

PlantDropAdapter::PlantDropAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity), m_rotation(0.0f) {
}

void PlantDropAdapter::setupComponents(
    List<Plant::PlantPiece> const& pieces,
    Vec2F const& pos,
    Vec2F const& strikeVector,
    String const& description,
    bool upsideDown,
    Json stemConfig,
    Json foliageConfig,
    Json saplingConfig,
    bool master,
    float random) {
  
  // Add tag
  addComponent<PlantDropTag>();
  
  // Entity type
  auto& entityType = addComponent<EntityTypeComponent>();
  entityType.type = EntityType::PlantDrop;
  entityType.ephemeral = true;
  
  // PlantDrop data
  auto& dropData = addComponent<PlantDropDataComponent>();
  dropData.description = description;
  dropData.stemConfig = stemConfig.isNull() ? JsonObject() : stemConfig;
  dropData.foliageConfig = foliageConfig.isNull() ? JsonObject() : foliageConfig;
  dropData.saplingConfig = saplingConfig;
  dropData.master = master;
  dropData.firstTick = true;
  dropData.spawnedDrops = false;
  dropData.spawnedDropEffects = false;
  dropData.time = 5000.0f;
  
  // Calculate rotation parameters
  if (!upsideDown) {
    dropData.rotationRate = copysign(0.00001f, -strikeVector[0] + random);
    dropData.rotationFallThreshold = Constants::pi / (3 + random);
    dropData.rotationCap = Constants::pi - dropData.rotationFallThreshold;
  } else {
    dropData.rotationRate = 0;
    dropData.rotationFallThreshold = 0;
    dropData.rotationCap = 0;
  }
  
  // Process pieces and calculate bounds
  bool structuralFound = false;
  RectF stemBounds = RectF::null();
  RectF fullBounds = RectF::null();
  
  for (auto& piece : pieces) {
    for (auto& spacePos : piece.spaces) {
      fullBounds.combine(Vec2F(spacePos));
      fullBounds.combine(Vec2F(spacePos) + Vec2F(1, 1));
      if (piece.structuralSegment) {
        structuralFound = true;
        stemBounds.combine(Vec2F(spacePos));
        stemBounds.combine(Vec2F(spacePos) + Vec2F(1, 1));
      }
    }
    
    PlantDropPiece pdp;
    pdp.image = piece.image;
    pdp.offset = piece.offset;
    pdp.segmentIdx = piece.segmentIdx;
    pdp.kind = piece.kind;
    pdp.flip = piece.flip;
    dropData.pieces.append(pdp);
  }
  
  if (fullBounds.isNull())
    fullBounds = RectF(pos, pos);
  if (stemBounds.isNull())
    stemBounds = RectF(pos, pos);
  
  dropData.boundingBox = fullBounds;
  dropData.collisionRect = structuralFound ? stemBounds : fullBounds;
  
  // Transform
  auto& transform = addComponent<TransformComponent>();
  transform.position = pos;
  
  // Velocity
  auto& velocity = addComponent<VelocityComponent>();
  velocity.velocity = Vec2F();
  
  // Bounds
  auto& bounds = addComponent<BoundsComponent>();
  bounds.metaBoundBox = dropData.boundingBox;
  bounds.collisionArea = dropData.collisionRect;
  
  // Physics
  auto& physics = addComponent<PhysicsBodyComponent>();
  physics.mass = 1.0f;
  physics.gravityMultiplier = 0.2f;
  physics.collisionEnabled = true;
  physics.gravityEnabled = true;
  
  // Network sync
  auto& netSync = addComponent<NetworkSyncComponent>();
  netSync.netVersion = 1;
  
  addComponent<InterpolationComponent>();
}

ByteArray PlantDropAdapter::netStore(NetCompatibilityRules rules) const {
  auto* dropData = getComponent<PlantDropDataComponent>();
  if (!dropData) return {};
  
  DataStreamBuffer ds;
  ds << dropData->time;
  ds << dropData->master;
  ds << dropData->description;
  ds << dropData->boundingBox;
  ds << dropData->collisionRect;
  ds << dropData->rotationRate;
  
  ds.writeContainer(dropData->pieces,
    [](DataStream& ds, PlantDropPiece const& piece) {
      ds.write(piece.image);
      ds.write(piece.offset[0]);
      ds.write(piece.offset[1]);
      ds.write(piece.flip);
      ds.write(piece.kind);
    });
  
  ds << dropData->stemConfig;
  ds << dropData->foliageConfig;
  ds << dropData->saplingConfig;
  
  return ds.data();
}

EntityType PlantDropAdapter::entityType() const {
  return EntityType::PlantDrop;
}

void PlantDropAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  EntityAdapter::init(world, entityId, mode);
  
  // The MovementController is handled by the ECS movement system now
}

void PlantDropAdapter::uninit() {
  EntityAdapter::uninit();
}

pair<ByteArray, uint64_t> PlantDropAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  DataStreamBuffer ds;
  
  auto* dropData = getComponent<PlantDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* netSync = getComponent<NetworkSyncComponent>();
  
  if (dropData && transform && velocity && netSync) {
    ds.write(transform->position);
    ds.write(velocity->velocity);
    ds.write(m_rotation);
    ds.write(dropData->spawnedDrops);
    ds.write(dropData->time);
    
    uint64_t version = netSync->netVersion;
    netSync->isDirty = false;
    return {ds.takeData(), version};
  }
  
  return {{}, 0};
}

void PlantDropAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  if (data.empty()) return;
  
  DataStreamBuffer ds(std::move(data));
  
  auto* dropData = getComponent<PlantDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (dropData && transform && velocity) {
    Vec2F newPos = ds.read<Vec2F>();
    Vec2F newVel = ds.read<Vec2F>();
    float newRotation = ds.read<float>();
    bool spawnedDrops = ds.read<bool>();
    float time = ds.read<float>();
    
    if (interp && interp->enabled) {
      interp->setTarget(newPos, newRotation);
      interp->interpolationTime = interpolationTime;
    } else {
      transform->position = newPos;
      m_rotation = newRotation;
    }
    velocity->velocity = newVel;
    dropData->spawnedDrops = spawnedDrops;
    dropData->time = time;
  }
}

void PlantDropAdapter::enableInterpolation(float extrapolationHint) {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp) {
    interp->enabled = true;
    interp->extrapolationHint = extrapolationHint;
  }
}

void PlantDropAdapter::disableInterpolation() {
  auto* interp = getComponent<InterpolationComponent>();
  if (interp) {
    interp->enabled = false;
  }
}

String PlantDropAdapter::description() const {
  auto* dropData = getComponent<PlantDropDataComponent>();
  return dropData ? dropData->description : "";
}

Vec2F PlantDropAdapter::position() const {
  auto* transform = getComponent<TransformComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (interp && interp->enabled) {
    return interp->interpolatedPosition();
  }
  return transform ? transform->position : Vec2F();
}

RectF PlantDropAdapter::metaBoundBox() const {
  auto* dropData = getComponent<PlantDropDataComponent>();
  return dropData ? dropData->boundingBox : RectF();
}

RectF PlantDropAdapter::collisionArea() const {
  return collisionRect();
}

RectF PlantDropAdapter::collisionRect() const {
  auto* dropData = getComponent<PlantDropDataComponent>();
  if (!dropData) return RectF();
  
  PolyF shape = PolyF(dropData->collisionRect);
  shape.rotate(m_rotation);
  return shape.boundBox();
}

float PlantDropAdapter::rotation() const {
  return m_rotation;
}

bool PlantDropAdapter::shouldDestroy() const {
  auto* dropData = getComponent<PlantDropDataComponent>();
  return dropData && dropData->time <= 0.0f;
}

void PlantDropAdapter::destroy(RenderCallback* renderCallback) {
  if (renderCallback) {
    render(renderCallback);
  }
}

void PlantDropAdapter::update(float dt, uint64_t currentStep) {
  auto* dropData = getComponent<PlantDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  auto* velocity = getComponent<VelocityComponent>();
  auto* physics = getComponent<PhysicsBodyComponent>();
  auto* interp = getComponent<InterpolationComponent>();
  
  if (!dropData || !transform || !velocity) return;
  
  dropData->time -= dt;
  
  bool isMaster = inWorld() && (world()->connection() == 0);
  
  if (isMaster) {
    // Handle false positive assumption for effects
    if (dropData->spawnedDropEffects && !dropData->spawnedDrops) {
      dropData->spawnedDropEffects = false;
    }
    if (dropData->spawnedDrops) {
      dropData->firstTick = false;
    }
    
    // Rotation physics
    float gravityMag = world()->gravity(transform->position);
    auto rotationAcceleration = 0.01f * gravityMag * copysign(1.0f, dropData->rotationRate) * dt;
    
    if (std::fabs(m_rotation) > dropData->rotationCap) {
      dropData->rotationRate -= rotationAcceleration;
    } else if (std::fabs(m_rotation) < dropData->rotationFallThreshold) {
      dropData->rotationRate += rotationAcceleration;
    }
    
    m_rotation += dropData->rotationRate;
    
    // Update gravity enabled state based on rotation
    if (physics) {
      physics->gravityEnabled = std::fabs(m_rotation) >= dropData->rotationFallThreshold;
    }
    
    if (dropData->time > 0) {
      // Check if on ground - simplified check
      // In full implementation, would use collision system
      CollisionComponent* collision = getComponent<CollisionComponent>();
      if (collision && collision->onGround) {
        dropData->time = 0;
      }
    }
    
    // Spawn drops when time is up or no gravity
    if ((dropData->time <= 0 || gravityMag == 0) && !dropData->spawnedDrops) {
      dropData->spawnedDrops = true;
      markNetworkDirty();
      
      auto imgMetadata = Root::singleton().imageMetadataDatabase();
      
      for (auto& piece : dropData->pieces) {
        JsonArray dropOptions;
        if (piece.kind == Plant::PlantPieceKind::Stem) {
          dropOptions = dropData->stemConfig.getArray("drops", {});
        }
        if (piece.kind == Plant::PlantPieceKind::Foliage) {
          dropOptions = dropData->foliageConfig.getArray("drops", {});
        }
        
        if (dropOptions.size()) {
          auto option = Random::randFrom(dropOptions).toArray();
          for (auto drop : option) {
            auto size = imgMetadata->imageSize(piece.image);
            Vec2F pos = Vec2F(piece.offset + Vec2F(size) * 0.5f / TilePixels);
            pos = pos.rotate(m_rotation);
            pos += Vec2F(Random::randf(-0.2f, 0.2f), Random::randf(-0.2f, 0.2f));
            
            if (drop.getString("item") == "sapling") {
              world()->addEntity(ItemDrop::createRandomizedDrop(
                ItemDescriptor("sapling", (size_t)drop.getInt("count", 1), dropData->saplingConfig),
                transform->position + pos));
            } else {
              world()->addEntity(ItemDrop::createRandomizedDrop(
                {drop.getString("item"), (size_t)drop.getInt("count", 1)},
                transform->position + pos));
            }
          }
        }
      }
    }
  } else {
    // Slave update
    if (interp && interp->enabled) {
      interp->update(dt, 10.0f);
    }
    
    if (dropData->spawnedDropEffects && !dropData->spawnedDrops) {
      dropData->spawnedDropEffects = false;
    }
    if (dropData->spawnedDrops) {
      dropData->firstTick = false;
    }
  }
}

void PlantDropAdapter::particleForPlantPart(
    PlantDropPiece const& piece,
    String const& mode,
    Json const& mainConfig,
    RenderCallback* renderCallback) {
  
  Json particleConfig = mainConfig.get("particles", JsonObject()).get(mode, JsonObject());
  JsonArray particleOptions = particleConfig.getArray("options", {});
  if (!particleOptions.size()) return;
  
  auto imgMetadata = Root::singleton().imageMetadataDatabase();
  
  Vec2F imageSize = Vec2F(imgMetadata->imageSize(piece.image)) / TilePixels;
  float density = (imageSize.x() * imageSize.y()) / particleConfig.getFloat("density", 1);
  
  auto spaces = Set<Vec2I>::from(imgMetadata->imageSpaces(piece.image, piece.offset * TilePixels, Plant::PlantScanThreshold, piece.flip));
  if (spaces.empty()) return;
  
  while (density > 0) {
    Vec2F particlePos = piece.offset + Vec2F(imageSize) / 2.0f + 
      Vec2F(Random::nrandf(imageSize.x() / 8.0f, 0), Random::nrandf(imageSize.y() / 8.0f, 0));
    
    if (!spaces.contains(Vec2I(particlePos.floor()))) continue;
    
    auto config = Random::randValueFrom(particleOptions, {});
    
    Particle particle = Root::singleton().particleDatabase()->particle(config);
    particle.color.hueShift(mainConfig.getFloat("hueshift", 0) / 360.0f);
    for (Directives const& directives : piece.image.directives.list()) {
      particle.directives.append(directives);
    }
    
    density--;
    
    particle.position = position() + particlePos.rotate(m_rotation);
    
    renderCallback->addParticle(particle);
  }
}

void PlantDropAdapter::render(RenderCallback* renderCallback) {
  auto* dropData = getComponent<PlantDropDataComponent>();
  auto* transform = getComponent<TransformComponent>();
  
  if (!dropData || !transform) return;
  
  auto assets = Root::singleton().assets();
  
  // First tick effects
  if (dropData->firstTick) {
    dropData->firstTick = false;
    
    if (dropData->master) {
      auto playBreakSound = [&](Json const& config) {
        JsonArray breakTreeOptions = config.get("sounds", JsonObject()).getArray("breakTree", JsonArray());
        if (breakTreeOptions.size()) {
          auto sound = Random::randFrom(breakTreeOptions);
          auto audioInstance = make_shared<AudioInstance>(*assets->audio(sound.getString("file")));
          audioInstance->setPosition(collisionRect().center() + position());
          audioInstance->setVolume(sound.getFloat("volume", 1.0f));
          renderCallback->addAudio(std::move(audioInstance));
        }
      };
      playBreakSound(dropData->stemConfig);
      playBreakSound(dropData->foliageConfig);
    }
    
    for (auto const& piece : dropData->pieces) {
      if (piece.kind == Plant::PlantPieceKind::Stem) {
        particleForPlantPart(piece, "breakTree", dropData->stemConfig, renderCallback);
      }
      if (piece.kind == Plant::PlantPieceKind::Foliage) {
        particleForPlantPart(piece, "breakTree", dropData->foliageConfig, renderCallback);
      }
    }
  }
  
  // Hit ground effects
  if (dropData->spawnedDrops && !dropData->spawnedDropEffects) {
    dropData->spawnedDropEffects = true;
    
    auto playHitSound = [&](Json const& config) {
      JsonArray hitGroundOptions = config.get("sounds", JsonObject()).getArray("hitGround", JsonArray());
      if (hitGroundOptions.size()) {
        auto sound = Random::randFrom(hitGroundOptions);
        auto audioInstance = make_shared<AudioInstance>(*assets->audio(sound.getString("file")));
        audioInstance->setPosition(collisionRect().center() + position());
        audioInstance->setVolume(sound.getFloat("volume", 1.0f));
        renderCallback->addAudio(std::move(audioInstance));
      }
    };
    playHitSound(dropData->stemConfig);
    playHitSound(dropData->foliageConfig);
    
    for (auto const& piece : dropData->pieces) {
      if (piece.kind == Plant::PlantPieceKind::Stem) {
        particleForPlantPart(piece, "hitGround", dropData->stemConfig, renderCallback);
      }
      if (piece.kind == Plant::PlantPieceKind::Foliage) {
        particleForPlantPart(piece, "hitGround", dropData->foliageConfig, renderCallback);
      }
    }
  }
  
  // Render pieces if still alive
  if (dropData->time > 0 && !dropData->spawnedDrops) {
    for (auto const& piece : dropData->pieces) {
      auto drawable = Drawable::makeImage(piece.image, 1.0f / TilePixels, false, piece.offset);
      if (piece.flip) {
        drawable.scale(Vec2F(-1, 1));
      }
      drawable.rotate(m_rotation);
      drawable.translate(position());
      renderCallback->addDrawable(std::move(drawable), RenderLayerPlantDrop);
    }
  }
}

void PlantDropAdapter::setPosition(Vec2F const& pos) {
  auto* transform = getComponent<TransformComponent>();
  if (transform) {
    transform->position = pos;
    markNetworkDirty();
  }
}

void PlantDropAdapter::setVelocity(Vec2F const& vel) {
  auto* velocity = getComponent<VelocityComponent>();
  if (velocity) {
    velocity->velocity = vel;
    markNetworkDirty();
  }
}

} // namespace ECS
} // namespace Star
