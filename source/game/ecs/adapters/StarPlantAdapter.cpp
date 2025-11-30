#include "StarPlantAdapter.hpp"
#include "StarWorld.hpp"
#include "StarRoot.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarAssets.hpp"
#include "StarPlantDrop.hpp"
#include "StarParticleDatabase.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarJsonExtra.hpp"

namespace Star {
namespace ECS {

float const PlantAdapter::PlantScanThreshold = 0.1f;

PlantPieceData::PlantPieceData() {
  image = "";
  imagePath = AssetPath();
  offset = {};
  segmentIdx = 0;
  structuralSegment = false;
  kind = PlantPieceKind::None;
  zLevel = 0.0f;
  rotationType = PlantRotationType::DontRotate;
  rotationOffset = 0.0f;
  spaces = {};
  flip = false;
}

shared_ptr<PlantAdapter> PlantAdapter::createTree(
    World* ecsWorld,
    TreeVariant const& config,
    uint64_t seed) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantAdapter>(ecsWorld, entity);
  
  // Add components
  adapter->addComponent<PlantTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  auto& data = adapter->addComponent<PlantDataComponent>();
  
  // Initialize from tree config
  data.broken = false;
  data.tilePosition = Vec2I();
  data.windTime = 0.0f;
  data.windLevel = 0.0f;
  data.ceiling = config.ceiling;
  data.piecesScanned = false;
  data.fallsWhenDead = true;
  data.piecesUpdated = true;
  data.tileDamageEvent = false;
  
  data.stemDropConfig = config.stemDropConfig;
  data.foliageDropConfig = config.foliageDropConfig;
  if (data.stemDropConfig.isNull())
    data.stemDropConfig = JsonObject();
  if (data.foliageDropConfig.isNull())
    data.foliageDropConfig = JsonObject();
  
  data.stemDropConfig = data.stemDropConfig.set("hueshift", config.stemHueShift);
  data.foliageDropConfig = data.foliageDropConfig.set("hueshift", config.foliageHueShift);
  
  JsonObject saplingDropConfig;
  saplingDropConfig["stemName"] = config.stemName;
  saplingDropConfig["stemHueShift"] = config.stemHueShift;
  if (!data.foliageDropConfig.isNull()) {
    saplingDropConfig["foliageName"] = config.foliageName;
    saplingDropConfig["foliageHueShift"] = config.foliageHueShift;
  }
  data.saplingDropConfig = saplingDropConfig;
  
  data.descriptions = config.descriptions;
  data.ephemeral = config.ephemeral;
  data.tileDamageParameters = config.tileDamageParameters;
  
  adapter->setupTreePieces(config, seed);
  adapter->setupNetStates();
  
  return adapter;
}

shared_ptr<PlantAdapter> PlantAdapter::createGrass(
    World* ecsWorld,
    GrassVariant const& config,
    uint64_t seed) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantAdapter>(ecsWorld, entity);
  
  adapter->addComponent<PlantTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  auto& data = adapter->addComponent<PlantDataComponent>();
  
  data.broken = false;
  data.tilePosition = Vec2I();
  data.ceiling = config.ceiling;
  data.windTime = 0.0f;
  data.windLevel = 0.0f;
  data.piecesScanned = false;
  data.fallsWhenDead = false;
  data.descriptions = config.descriptions;
  data.ephemeral = config.ephemeral;
  data.tileDamageParameters = config.tileDamageParameters;
  data.piecesUpdated = true;
  
  adapter->setupGrassPieces(config, seed);
  adapter->setupNetStates();
  
  return adapter;
}

shared_ptr<PlantAdapter> PlantAdapter::createBush(
    World* ecsWorld,
    BushVariant const& config,
    uint64_t seed) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantAdapter>(ecsWorld, entity);
  
  adapter->addComponent<PlantTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  auto& data = adapter->addComponent<PlantDataComponent>();
  
  data.broken = false;
  data.tilePosition = Vec2I();
  data.ceiling = config.ceiling;
  data.windTime = 0.0f;
  data.windLevel = 0.0f;
  data.piecesScanned = false;
  data.fallsWhenDead = false;
  data.descriptions = config.descriptions;
  data.ephemeral = config.ephemeral;
  data.tileDamageParameters = config.tileDamageParameters;
  data.piecesUpdated = true;
  
  adapter->setupBushPieces(config, seed);
  adapter->setupNetStates();
  
  return adapter;
}

shared_ptr<PlantAdapter> PlantAdapter::createFromDiskStore(
    World* ecsWorld,
    Json const& diskStore) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantAdapter>(ecsWorld, entity);
  
  adapter->addComponent<PlantTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  auto& data = adapter->addComponent<PlantDataComponent>();
  
  data.tilePosition = jsonToVec2I(diskStore.get("tilePosition"));
  data.ceiling = diskStore.getBool("ceiling");
  data.stemDropConfig = diskStore.get("stemDropConfig");
  data.foliageDropConfig = diskStore.get("foliageDropConfig");
  data.saplingDropConfig = diskStore.get("saplingDropConfig");
  data.descriptions = diskStore.get("descriptions");
  data.ephemeral = diskStore.getBool("ephemeral");
  data.tileDamageParameters = TileDamageParameters(diskStore.get("tileDamageParameters"));
  data.fallsWhenDead = diskStore.getBool("fallsWhenDead");
  adapter->readPiecesFromJson(diskStore.get("pieces"));
  
  adapter->setupNetStates();
  
  return adapter;
}

shared_ptr<PlantAdapter> PlantAdapter::createFromNet(
    World* ecsWorld,
    ByteArray const& netStore,
    NetCompatibilityRules rules) {
  Entity entity = ecsWorld->createEntity();
  auto adapter = make_shared<PlantAdapter>(ecsWorld, entity);
  
  adapter->addComponent<PlantTag>();
  adapter->addComponent<PositionComponent>(Vec2F());
  adapter->addComponent<BoundingBoxComponent>(RectF());
  auto& data = adapter->addComponent<PlantDataComponent>();
  
  data.broken = false;
  data.windTime = 0.0f;
  data.windLevel = 0.0f;
  data.piecesScanned = false;
  data.piecesUpdated = true;
  
  DataStreamBuffer ds(netStore);
  ds.setStreamCompatibilityVersion(rules);
  ds.viread(data.tilePosition[0]);
  ds.viread(data.tilePosition[1]);
  ds.read(data.ceiling);
  ds.read(data.stemDropConfig);
  ds.read(data.foliageDropConfig);
  ds.read(data.saplingDropConfig);
  ds.read(data.descriptions);
  ds.read(data.ephemeral);
  ds.read(data.tileDamageParameters);
  ds.read(data.fallsWhenDead);
  data.tileDamageStatus.netLoad(ds, rules);
  adapter->readPieces(ds.read<ByteArray>());
  
  adapter->setupNetStates();
  
  return adapter;
}

PlantAdapter::PlantAdapter(World* ecsWorld, Entity ecsEntity)
  : EntityAdapter(ecsWorld, ecsEntity) {
}

Json PlantAdapter::diskStore() const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return {};
    
  return JsonObject{
    {"tilePosition", jsonFromVec2I(data->tilePosition)},
    {"ceiling", data->ceiling},
    {"stemDropConfig", data->stemDropConfig},
    {"foliageDropConfig", data->foliageDropConfig},
    {"saplingDropConfig", data->saplingDropConfig},
    {"descriptions", data->descriptions},
    {"ephemeral", data->ephemeral},
    {"tileDamageParameters", data->tileDamageParameters.toJson()},
    {"fallsWhenDead", data->fallsWhenDead},
    {"pieces", writePiecesToJson()},
  };
}

ByteArray PlantAdapter::netStore(NetCompatibilityRules rules) const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return {};
    
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);
  ds.viwrite(data->tilePosition[0]);
  ds.viwrite(data->tilePosition[1]);
  ds.write(data->ceiling);
  ds.write(data->stemDropConfig);
  ds.write(data->foliageDropConfig);
  ds.write(data->saplingDropConfig);
  ds.write(data->descriptions);
  ds.write(data->ephemeral);
  ds.write(data->tileDamageParameters);
  ds.write(data->fallsWhenDead);
  data->tileDamageStatus.netStore(ds, rules);
  ds.write(writePieces());
  
  return ds.takeData();
}

EntityType PlantAdapter::entityType() const {
  return EntityType::Plant;
}

void PlantAdapter::init(Star::World* world, EntityId entityId, EntityMode mode) {
  Entity::init(world, entityId, mode);
  validatePieces();
  
  auto* data = getComponent<PlantDataComponent>();
  if (data) {
    data->tilePosition = world->geometry().xwrap(data->tilePosition);
    
    // Update position component
    auto* pos = getComponent<PositionComponent>();
    if (pos)
      pos->position = Vec2F(data->tilePosition);
  }
}

void PlantAdapter::uninit() {
  Entity::uninit();
}

String PlantAdapter::description() const {
  auto* data = getComponent<PlantDataComponent>();
  if (data)
    return data->descriptions.getString("description", "");
  return "";
}

pair<ByteArray, uint64_t> PlantAdapter::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void PlantAdapter::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

void PlantAdapter::enableInterpolation(float extrapolationHint) {
  auto* data = getComponent<PlantDataComponent>();
  if (data && data->fallsWhenDead)
    m_netGroup.enableNetInterpolation(extrapolationHint);
}

void PlantAdapter::disableInterpolation() {
  m_netGroup.disableNetInterpolation();
}

Vec2F PlantAdapter::position() const {
  auto* data = getComponent<PlantDataComponent>();
  if (data)
    return Vec2F(data->tilePosition);
  return Vec2F();
}

RectF PlantAdapter::metaBoundBox() const {
  auto* data = getComponent<PlantDataComponent>();
  if (data)
    return data->metaBoundBox;
  return RectF();
}

bool PlantAdapter::ephemeral() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? data->ephemeral : false;
}

bool PlantAdapter::shouldDestroy() const {
  auto* data = getComponent<PlantDataComponent>();
  if (data)
    return data->broken || data->pieces.size() == 0;
  return true;
}

Vec2I PlantAdapter::tilePosition() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? data->tilePosition : Vec2I();
}

void PlantAdapter::setTilePosition(Vec2I const& tilePosition) {
  auto* data = getComponent<PlantDataComponent>();
  if (data) {
    data->tilePosition = tilePosition;
    
    auto* pos = getComponent<PositionComponent>();
    if (pos)
      pos->position = Vec2F(tilePosition);
  }
}

List<Vec2I> PlantAdapter::spaces() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? data->spaces : List<Vec2I>();
}

List<Vec2I> PlantAdapter::roots() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? data->roots : List<Vec2I>();
}

bool PlantAdapter::checkBroken() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return true;
    
  if (!data->broken) {
    if (!allSpacesOccupied(data->roots)) {
      if (data->fallsWhenDead) {
        breakAtPosition(data->tilePosition, Vec2F(data->tilePosition));
        return false;
      } else {
        data->broken = true;
      }
    } else if (anySpacesOccupied(data->spaces)) {
      data->broken = true;
    }
  }
  
  return data->broken;
}

bool PlantAdapter::damageTiles(List<Vec2I> const& positions, Vec2F const& sourcePosition, TileDamage const& tileDamage) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return false;
    
  auto pos = baseDamagePosition(positions);
  auto geometry = world()->geometry();
  
  data->tileDamageStatus.damage(data->tileDamageParameters, tileDamage);
  data->tileDamageX = geometry.diff(pos[0], data->tilePosition[0]);
  data->tileDamageY = pos[1] - data->tilePosition[1];
  data->tileDamageEvent = true;
  data->tileDamageEventTrigger = true;
  
  bool breaking = false;
  if (data->tileDamageStatus.dead()) {
    breaking = true;
    if (data->fallsWhenDead) {
      data->tileDamageStatus.reset();
      breakAtPosition(pos, sourcePosition);
    } else {
      data->broken = true;
    }
  }
  
  return breaking;
}

RectF PlantAdapter::interactiveBoundBox() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? RectF(data->boundBox) : RectF();
}

Vec2I PlantAdapter::primaryRoot() const {
  auto* data = getComponent<PlantDataComponent>();
  if (data)
    return data->ceiling ? Vec2I(0, 1) : Vec2I(0, -1);
  return Vec2I(0, -1);
}

bool PlantAdapter::ceiling() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? data->ceiling : false;
}

List<PlantPieceData> PlantAdapter::pieces() const {
  auto* data = getComponent<PlantDataComponent>();
  return data ? data->pieces : List<PlantPieceData>();
}

void PlantAdapter::update(float dt, uint64_t) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  data->windTime += dt;
  data->windTime = std::fmod(data->windTime, 628.32f);
  data->windLevel = world()->windLevel(Vec2F(data->tilePosition));
  
  if (isMaster()) {
    if (data->tileDamageStatus.damaged())
      data->tileDamageStatus.recover(data->tileDamageParameters, dt);
  } else {
    if (data->tileDamageStatus.damaged() && !data->tileDamageStatus.damageProtected()) {
      float damageEffectPercentage = data->tileDamageStatus.damageEffectPercentage();
      data->windTime += damageEffectPercentage * 10 * dt;
      data->windLevel += damageEffectPercentage * 20;
    }
    
    m_netGroup.tickNetInterpolation(dt);
  }
}

void PlantAdapter::render(RenderCallback* renderCallback) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  float damageXOffset = Random::randf(-0.1f, 0.1f) * data->tileDamageStatus.damageEffectPercentage();
  
  for (auto const& piece : data->pieces) {
    auto size = Vec2F(piece.imageSize) / TilePixels;
    
    Vec2F offset = piece.offset;
    if ((data->ceiling && offset[1] <= data->tileDamageY) || 
        (!data->ceiling && offset[1] + size[1] >= data->tileDamageY))
      offset[0] += damageXOffset;
    
    auto drawable = Drawable::makeImage(piece.imagePath, 1.0f / TilePixels, false, offset);
    if (piece.flip)
      drawable.scale(Vec2F(-1, 1));
    
    if (piece.rotationType == PlantRotationType::RotateCrownBranch || 
        piece.rotationType == PlantRotationType::RotateCrownLeaves) {
      drawable.rotate(branchRotation(data->tilePosition[0], piece.rotationOffset * 1.4f) * 0.7f, 
                     piece.offset + Vec2F(size[0] / 2.0f, 0));
      drawable.translate(Vec2F(0, -0.40f));
    } else if (piece.rotationType == PlantRotationType::RotateBranch || 
               piece.rotationType == PlantRotationType::RotateLeaves) {
      drawable.rotate(branchRotation(data->tilePosition[0], piece.rotationOffset * 1.4f), 
                     piece.offset + Vec2F(size) / 2.0f);
    }
    drawable.translate(position());
    renderCallback->addDrawable(std::move(drawable), RenderLayerPlant);
  }
  
  if (data->tileDamageEvent) {
    data->tileDamageEvent = false;
    if (data->stemDropConfig.type() == Json::Type::Object) {
      Json particleConfig = data->stemDropConfig.get("particles", JsonObject()).get("damageTree", JsonObject());
      JsonArray particleOptions = particleConfig.getArray("options", {});
      auto hueshift = data->stemDropConfig.getFloat("hueshift", 0) / 360.0f;
      auto density = particleConfig.getFloat("density", 1);
      while (density-- > 0) {
        auto config = Random::randValueFrom(particleOptions, {});
        if (config.isNull() || config.size() == 0)
          continue;
        auto particle = Root::singleton().particleDatabase()->particle(config);
        particle.color.hueShift(hueshift);
        if (!particle.string.empty()) {
          particle.string = strf("{}?hueshift={}", particle.string, hueshift);
          particle.image = particle.string;
        }
        particle.position = {data->tileDamageX + Random::randf(), data->tileDamageY + Random::randf()};
        particle.translate(position());
        renderCallback->addParticle(std::move(particle));
      }
      JsonArray damageTreeSoundOptions = data->stemDropConfig.get("sounds", JsonObject()).getArray("damageTree", JsonArray());
      if (damageTreeSoundOptions.size()) {
        auto sound = Random::randFrom(damageTreeSoundOptions);
        Vec2F pos = position() + Vec2F(data->tileDamageX + Random::randf(), data->tileDamageY + Random::randf());
        auto assets = Root::singleton().assets();
        auto audioInstance = make_shared<AudioInstance>(*assets->audio(sound.getString("file")));
        audioInstance->setPosition(pos);
        audioInstance->setVolume(sound.getFloat("volume", 1.0f));
        renderCallback->addAudio(std::move(audioInstance));
      }
    }
  }
}

void PlantAdapter::setupTreePieces(TreeVariant const& config, uint64_t seed) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  RandomSource rnd(seed);
  
  float xOffset = 0;
  float yOffset = 0;
  float roffset = Random::randf() * 0.5f;
  int segment = 0;
  
  auto assets = Root::singleton().assets();
  
  // Base
  {
    JsonObject bases = config.stemSettings.get("base").toObject();
    String baseKey = bases.keys()[rnd.randInt(bases.size() - 1)];
    JsonObject baseSettings = bases[baseKey].toObject();
    JsonObject attachmentSettings = baseSettings["attachment"].toObject();
    
    xOffset += attachmentSettings.get("bx").toDouble() / TilePixels;
    yOffset += attachmentSettings.get("by").toDouble() / TilePixels;
    
    String baseFile = AssetPath::relativeTo(config.stemDirectory, baseSettings.get("image").toString());
    float baseImageHeight = assets->image(baseFile)->height();
    if (config.ceiling)
      yOffset = 1.0 - baseImageHeight / TilePixels;
    
    {
      PlantPieceData piece;
      piece.image = strf("{}?hueshift={}", baseFile, config.stemHueShift);
      piece.offset = Vec2F(xOffset, yOffset);
      piece.segmentIdx = segment;
      piece.structuralSegment = true;
      piece.kind = PlantPieceKind::Stem;
      piece.zLevel = 0.0f;
      piece.rotationType = PlantRotationType::DontRotate;
      piece.rotationOffset = Random::randf() + roffset;
      data->pieces.append(piece);
    }
    
    // Base leaves
    JsonObject baseLeaves = config.foliageSettings.getObject("baseLeaves", {});
    if (baseLeaves.contains(baseKey)) {
      JsonObject baseLeavesSettings = baseLeaves.get(baseKey).toObject();
      JsonObject attachSettings = baseLeavesSettings["attachment"].toObject();
      
      float xOf = xOffset + attachSettings.get("bx").toDouble() / TilePixels;
      float yOf = yOffset + attachSettings.get("by").toDouble() / TilePixels;
      
      if (baseLeavesSettings.contains("image") && !baseLeavesSettings.get("image").toString().empty()) {
        String baseLeavesFile = AssetPath::relativeTo(config.foliageDirectory, baseLeavesSettings.get("image").toString());
        
        PlantPieceData piece;
        piece.image = strf("{}?hueshift={}", baseLeavesFile, config.foliageHueShift);
        piece.offset = Vec2F{xOf, yOf};
        piece.segmentIdx = segment;
        piece.structuralSegment = false;
        piece.kind = PlantPieceKind::Foliage;
        piece.zLevel = 3.0f;
        piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateLeaves;
        piece.rotationOffset = Random::randf() + roffset;
        data->pieces.append(piece);
      }
      
      if (baseLeavesSettings.contains("backimage") && !baseLeavesSettings.get("backimage").toString().empty()) {
        String baseLeavesBackFile = AssetPath::relativeTo(config.foliageDirectory, baseLeavesSettings.get("backimage").toString());
        PlantPieceData piece;
        piece.image = strf("{}?hueshift={}", baseLeavesBackFile, config.foliageHueShift);
        piece.offset = Vec2F{xOf, yOf};
        piece.segmentIdx = segment;
        piece.structuralSegment = false;
        piece.kind = PlantPieceKind::Foliage;
        piece.zLevel = -1.0f;
        piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateLeaves;
        piece.rotationOffset = Random::randf() + roffset;
        data->pieces.append(piece);
      }
    }
    
    xOffset += attachmentSettings.get("x").toDouble() / TilePixels;
    yOffset += attachmentSettings.get("y").toDouble() / TilePixels;
    
    segment++;
  }
  
  float branchYOffset = yOffset;
  
  // Trunk
  {
    JsonObject middles = config.stemSettings.get("middle").toObject();
    int middleHeight = config.stemSettings.getInt("middleMinSize", 1) + 
                       rnd.randInt(config.stemSettings.getInt("middleMaxSize", 6) - 
                                  config.stemSettings.getInt("middleMinSize", 1));
    
    bool hasBranches = config.stemSettings.contains("branch");
    JsonObject branches;
    if (hasBranches) {
      branches = config.stemSettings.get("branch").toObject();
      if (branches.size() == 0)
        hasBranches = false;
    }
    
    for (int i = 0; i < middleHeight; i++) {
      String middleKey = middles.keys()[rnd.randInt(middles.size() - 1)];
      JsonObject middleSettings = middles[middleKey].toObject();
      JsonObject attachmentSettings = middleSettings["attachment"].toObject();
      
      xOffset += attachmentSettings.get("bx").toDouble() / TilePixels;
      yOffset += attachmentSettings.get("by").toDouble() / TilePixels;
      
      String middleFile = AssetPath::relativeTo(config.stemDirectory, middleSettings.get("image").toString());
      
      {
        PlantPieceData piece;
        piece.image = strf("{}?hueshift={}", middleFile, config.stemHueShift);
        piece.offset = Vec2F(xOffset, yOffset);
        piece.segmentIdx = segment;
        piece.structuralSegment = true;
        piece.kind = PlantPieceKind::Stem;
        piece.zLevel = 1.0f;
        piece.rotationType = PlantRotationType::DontRotate;
        piece.rotationOffset = Random::randf() + roffset;
        data->pieces.append(piece);
      }
      
      // Trunk leaves
      JsonObject trunkLeaves = config.foliageSettings.getObject("trunkLeaves", {});
      if (trunkLeaves.contains(middleKey)) {
        JsonObject trunkLeavesSettings = trunkLeaves.get(middleKey).toObject();
        JsonObject attachSettings = trunkLeavesSettings["attachment"].toObject();
        
        float xOf = xOffset + attachSettings.get("bx").toDouble() / TilePixels;
        float yOf = yOffset + attachSettings.get("by").toDouble() / TilePixels;
        
        if (trunkLeavesSettings.contains("image") && !trunkLeavesSettings.get("image").toString().empty()) {
          String trunkLeavesFile = AssetPath::relativeTo(config.foliageDirectory, trunkLeavesSettings.get("image").toString());
          PlantPieceData piece;
          piece.image = strf("{}?hueshift={}", trunkLeavesFile, config.foliageHueShift);
          piece.offset = Vec2F{xOf, yOf};
          piece.segmentIdx = segment;
          piece.structuralSegment = false;
          piece.kind = PlantPieceKind::Foliage;
          piece.zLevel = 3.0f;
          piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateLeaves;
          piece.rotationOffset = Random::randf() + roffset;
          data->pieces.append(piece);
        }
        
        if (trunkLeavesSettings.contains("backimage") && !trunkLeavesSettings.get("backimage").toString().empty()) {
          String trunkLeavesBackFile = AssetPath::relativeTo(config.foliageDirectory, trunkLeavesSettings.get("backimage").toString());
          PlantPieceData piece;
          piece.image = strf("{}?hueshift={}", trunkLeavesBackFile, config.foliageHueShift);
          piece.offset = Vec2F{xOf, yOf};
          piece.segmentIdx = segment;
          piece.structuralSegment = false;
          piece.kind = PlantPieceKind::Foliage;
          piece.zLevel = -1.0f;
          piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateLeaves;
          piece.rotationOffset = Random::randf() + roffset;
          data->pieces.append(piece);
        }
      }
      
      xOffset += attachmentSettings.get("x").toDouble() / TilePixels;
      yOffset += attachmentSettings.get("y").toDouble() / TilePixels;
      
      // Branches
      while (hasBranches && (yOffset >= branchYOffset) && ((middleHeight - i) > 0)) {
        String branchKey = branches.keys()[rnd.randInt(branches.size() - 1)];
        JsonObject branchSettings = branches[branchKey].toObject();
        JsonObject branchAttach = branchSettings["attachment"].toObject();
        
        float h = branchAttach.get("h").toDouble() / TilePixels;
        if (yOffset < branchYOffset + (h / 2.0f))
          break;
        
        float xO = xOffset + branchAttach.get("bx").toDouble() / TilePixels;
        float yO = branchYOffset + branchAttach.get("by").toDouble() / TilePixels;
        
        if (config.stemSettings.getBool("alwaysBranch", false) || rnd.randInt(2 + i) != 0) {
          float boffset = Random::randf() + roffset;
          String branchFile = AssetPath::relativeTo(config.stemDirectory, branchSettings.get("image").toString());
          {
            PlantPieceData piece;
            piece.image = strf("{}?hueshift={}", branchFile, config.stemHueShift);
            piece.offset = Vec2F{xO, yO};
            piece.segmentIdx = segment;
            piece.structuralSegment = false;
            piece.kind = PlantPieceKind::Stem;
            piece.zLevel = 0.0f;
            piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateBranch;
            piece.rotationOffset = boffset;
            data->pieces.append(piece);
          }
          branchYOffset += h;
          
          // Branch leaves
          JsonObject branchLeaves = config.foliageSettings.getObject("branchLeaves", {});
          if (branchLeaves.contains(branchKey)) {
            JsonObject branchLeavesSettings = branchLeaves.get(branchKey).toObject();
            JsonObject leafAttach = branchLeavesSettings["attachment"].toObject();
            
            float xOf = xO + leafAttach.get("bx").toDouble() / TilePixels;
            float yOf = yO + leafAttach.get("by").toDouble() / TilePixels;
            
            if (branchLeavesSettings.contains("image") && !branchLeavesSettings.get("image").toString().empty()) {
              String branchLeavesFile = AssetPath::relativeTo(config.foliageDirectory, branchLeavesSettings.get("image").toString());
              PlantPieceData piece;
              piece.image = strf("{}?hueshift={}", branchLeavesFile, config.foliageHueShift);
              piece.offset = Vec2F{xOf, yOf};
              piece.segmentIdx = segment;
              piece.structuralSegment = false;
              piece.kind = PlantPieceKind::Foliage;
              piece.zLevel = 3.0f;
              piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateLeaves;
              piece.rotationOffset = boffset;
              data->pieces.append(piece);
            }
            
            if (branchLeavesSettings.contains("backimage") && !branchLeavesSettings.get("backimage").toString().empty()) {
              String branchLeavesBackFile = AssetPath::relativeTo(config.foliageDirectory, branchLeavesSettings.get("backimage").toString());
              PlantPieceData piece;
              piece.image = strf("{}?hueshift={}", branchLeavesBackFile, config.foliageHueShift);
              piece.offset = Vec2F{xOf, yOf};
              piece.segmentIdx = segment;
              piece.structuralSegment = false;
              piece.kind = PlantPieceKind::Foliage;
              piece.zLevel = -1.0f;
              piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateLeaves;
              piece.rotationOffset = boffset;
              data->pieces.append(piece);
            }
          }
        } else {
          branchYOffset += (branchAttach.get("h").toDouble() / TilePixels) / (float)(1 + rnd.randInt(4));
        }
      }
      segment++;
    }
  }
  
  // Crown
  {
    JsonObject crowns = config.stemSettings.getObject("crown", {});
    bool hasCrown = crowns.size() > 0;
    if (hasCrown) {
      String crownKey = crowns.keys()[rnd.randInt(crowns.size() - 1)];
      JsonObject crownSettings = crowns[crownKey].toObject();
      JsonObject attachmentSettings = crownSettings["attachment"].toObject();
      
      xOffset += attachmentSettings.get("bx").toDouble() / TilePixels;
      yOffset += attachmentSettings.get("by").toDouble() / TilePixels;
      
      float coffset = roffset + Random::randf();
      
      String crownFile = AssetPath::relativeTo(config.stemDirectory, crownSettings.get("image").toString());
      {
        PlantPieceData piece;
        piece.image = strf("{}?hueshift={}", crownFile, config.stemHueShift);
        piece.offset = Vec2F{xOffset, yOffset};
        piece.segmentIdx = segment;
        piece.structuralSegment = false;
        piece.kind = PlantPieceKind::Stem;
        piece.zLevel = 0.0f;
        piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateCrownBranch;
        piece.rotationOffset = coffset;
        data->pieces.append(piece);
      }
      
      // Crown leaves
      JsonObject crownLeaves = config.foliageSettings.getObject("crownLeaves", {});
      if (crownLeaves.contains(crownKey)) {
        JsonObject crownLeavesSettings = crownLeaves.get(crownKey).toObject();
        JsonObject leafAttach = crownLeavesSettings["attachment"].toObject();
        
        float xO = xOffset + leafAttach.get("bx").toDouble() / TilePixels;
        float yO = yOffset + leafAttach.get("by").toDouble() / TilePixels;
        
        if (crownLeavesSettings.contains("image") && !crownLeavesSettings.get("image").toString().empty()) {
          String crownLeavesFile = AssetPath::relativeTo(config.foliageDirectory, crownLeavesSettings.get("image").toString());
          
          PlantPieceData piece;
          piece.image = strf("{}?hueshift={}", crownLeavesFile, config.foliageHueShift);
          piece.offset = Vec2F{xO, yO};
          piece.segmentIdx = segment;
          piece.structuralSegment = false;
          piece.kind = PlantPieceKind::Foliage;
          piece.zLevel = 3.0f;
          piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateCrownLeaves;
          piece.rotationOffset = coffset;
          data->pieces.append(piece);
        }
        
        if (crownLeavesSettings.contains("backimage") && !crownLeavesSettings.get("backimage").toString().empty()) {
          String crownLeavesBackFile = AssetPath::relativeTo(config.foliageDirectory, crownLeavesSettings.get("backimage").toString());
          
          PlantPieceData piece;
          piece.image = strf("{}?hueshift={}", crownLeavesBackFile, config.foliageHueShift);
          piece.offset = Vec2F{xO, yO};
          piece.segmentIdx = segment;
          piece.structuralSegment = false;
          piece.kind = PlantPieceKind::Foliage;
          piece.zLevel = -1.0f;
          piece.rotationType = data->ceiling ? PlantRotationType::DontRotate : PlantRotationType::RotateCrownLeaves;
          piece.rotationOffset = coffset;
          data->pieces.append(piece);
        }
      }
    }
  }
  
  sort(data->pieces, [](PlantPieceData const& a, PlantPieceData const& b) { return a.zLevel < b.zLevel; });
  validatePieces();
}

void PlantAdapter::setupGrassPieces(GrassVariant const& config, uint64_t seed) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  RandomSource rand(seed);
  
  String imageName = AssetPath::relativeTo(config.directory, rand.randValueFrom(config.images));
  
  Vec2F offset = Vec2F();
  if (config.ceiling) {
    auto imgMetadata = Root::singleton().imageMetadataDatabase();
    float imageHeight = imgMetadata->imageSize(imageName)[1];
    offset = Vec2F(0.0f, 1.0f - imageHeight / TilePixels);
  }
  
  PlantPieceData piece;
  piece.image = strf("{}?hueshift={}", imageName, config.hueShift);
  piece.offset = offset;
  piece.segmentIdx = 0;
  piece.structuralSegment = true;
  piece.kind = PlantPieceKind::None;
  data->pieces = {piece};
  
  validatePieces();
}

void PlantAdapter::setupBushPieces(BushVariant const& config, uint64_t seed) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  RandomSource rand(seed);
  auto assets = Root::singleton().assets();
  
  auto shape = rand.randValueFrom(config.shapes);
  String shapeImageName = AssetPath::relativeTo(config.directory, shape.image);
  float shapeImageHeight = assets->image(shapeImageName)->height();
  Vec2F offset = Vec2F();
  if (config.ceiling)
    offset = Vec2F(0.0f, 1.0f - shapeImageHeight / TilePixels);
  
  {
    PlantPieceData piece;
    piece.image = strf("{}?hueshift={}", shapeImageName, config.baseHueShift);
    piece.offset = offset;
    piece.segmentIdx = 0;
    piece.structuralSegment = true;
    piece.kind = PlantPieceKind::None;
    data->pieces.append(piece);
  }
  
  auto mod = rand.randValueFrom(shape.mods);
  if (!mod.empty()) {
    PlantPieceData piece;
    piece.image = strf("{}?hueshift={}", AssetPath::relativeTo(config.directory, mod), config.modHueShift);
    piece.offset = offset;
    piece.segmentIdx = 0;
    piece.structuralSegment = false;
    piece.kind = PlantPieceKind::None;
    data->pieces.append(piece);
  }
  
  validatePieces();
}

void PlantAdapter::breakAtPosition(Vec2I const& position, Vec2F const& sourcePosition) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  auto geometry = world()->geometry();
  Vec2I internalPos = geometry.diff(position, data->tilePosition);
  size_t idx = highest<size_t>();
  int segmentIdx = highest<int>();
  for (size_t i = 0; i < data->pieces.size(); ++i) {
    auto& piece = data->pieces[i];
    if (piece.structuralSegment && piece.spaces.contains(internalPos)) {
      if (piece.segmentIdx < segmentIdx) {
        segmentIdx = piece.segmentIdx;
        idx = i;
      }
    }
  }
  
  // Default to highest structural piece
  if (idx >= data->pieces.size()) {
    for (size_t i = data->pieces.size(); i > 0; --i) {
      auto& piece = data->pieces[i - 1];
      if (piece.structuralSegment) {
        segmentIdx = piece.segmentIdx;
        idx = i - 1;
        break;
      }
    }
  }
  
  // Plant has no structural segments
  if (idx >= data->pieces.size())
    return;
  
  PlantPieceData breakPiece = data->pieces[idx];
  Vec2F breakPoint = Vec2F(position) - Vec2F(data->tilePosition);
  if (breakPiece.spaces.size()) {
    RectF bounds = RectF::null();
    for (auto space : breakPiece.spaces) {
      bounds.combine(Vec2F(space));
      bounds.combine(Vec2F(space) + Vec2F(1, 1));
    }
    breakPoint[0] = (bounds.max()[0] + bounds.min()[0]) / 2.0f;
    if (!data->ceiling)
      breakPoint[1] = bounds.min()[1];
    else
      breakPoint[1] = bounds.max()[1];
  }
  
  List<PlantPieceData> droppedPieces;
  if (data->pieces[idx].structuralSegment) {
    idx = 0;
    while (idx < data->pieces.size()) {
      if (data->pieces[idx].segmentIdx >= segmentIdx) {
        droppedPieces.append(data->pieces.takeAt(idx));
        continue;
      }
      idx++;
    }
  } else {
    droppedPieces.append(data->pieces.takeAt(idx));
  }
  data->piecesUpdated = true;
  
  Vec2I breakPointI = Vec2I(round(breakPoint[0]), round(breakPoint[1]));
  
  // Calculate a new origin for the droppedPieces
  for (auto& piece : droppedPieces) {
    piece.offset -= breakPoint;
    Set<Vec2I> spaces = piece.spaces;
    piece.spaces.clear();
    for (auto const& space : spaces) {
      piece.spaces.add(space - breakPointI);
    }
  }
  
  Vec2F worldSpaceBreakPoint = breakPoint + Vec2F(data->tilePosition);
  
  // Convert PlantPieceData to Plant::PlantPiece for PlantDrop
  auto convertPiece = [](PlantPieceData const& src) -> Plant::PlantPiece {
    Plant::PlantPiece dst;
    dst.imagePath = src.imagePath;
    dst.image = src.image;
    dst.imageSize = src.imageSize;
    dst.offset = src.offset;
    dst.segmentIdx = src.segmentIdx;
    dst.structuralSegment = src.structuralSegment;
    dst.kind = static_cast<Plant::PlantPieceKind>(src.kind);
    dst.rotationType = static_cast<Plant::RotationType>(src.rotationType);
    dst.rotationOffset = src.rotationOffset;
    dst.spaces = src.spaces;
    dst.flip = src.flip;
    dst.zLevel = src.zLevel;
    return dst;
  };
  
  List<int> segmentOrder;
  Map<int, List<Plant::PlantPiece>> segments;
  for (auto& piece : droppedPieces) {
    if (!segments.contains(piece.segmentIdx))
      segmentOrder.append(piece.segmentIdx);
    segments[piece.segmentIdx].append(convertPiece(piece));
  }
  reverse(segmentOrder);
  float random = Random::randf(-0.3f, 0.3f);
  auto fallVector = (worldSpaceBreakPoint - sourcePosition).normalized();
  bool first = true;
  for (auto seg : segmentOrder) {
    auto segment = segments[seg];
    world()->addEntity(make_shared<PlantDrop>(segment,
        worldSpaceBreakPoint,
        fallVector,
        description(),
        data->ceiling,
        data->stemDropConfig,
        data->foliageDropConfig,
        data->saplingDropConfig,
        first,
        random));
    first = false;
  }
  
  data->piecesScanned = false;
  validatePieces();
}

Vec2I PlantAdapter::baseDamagePosition(List<Vec2I> const& positions) const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data || positions.empty())
    return positions.empty() ? Vec2I() : positions[0];
    
  auto res = positions.at(0);
  
  for (auto const& piece : data->pieces) {
    if (piece.structuralSegment) {
      for (auto space : piece.spaces) {
        for (auto position : positions) {
          if (world()->geometry().equal(data->tilePosition + space, position)) {
            if ((res[1] < position[1]) == data->ceiling) {
              res = position;
            }
          }
        }
      }
    }
  }
  
  return res;
}

bool PlantAdapter::damagable() const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return true;
  if (data->stemDropConfig.type() != Json::Type::Object)
    return true;
  if (!data->stemDropConfig.getBool("destructable", true))
    return false;
  return true;
}

void PlantAdapter::scanSpacesAndRoots() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  auto imageMetadataDatabase = Root::singleton().imageMetadataDatabase();
  
  Set<Vec2I> spaces;
  spaces.add({0, 0});
  
  for (auto& piece : data->pieces) {
    piece.imageSize = imageMetadataDatabase->imageSize(piece.image);
    piece.spaces = Set<Vec2I>::from(
        imageMetadataDatabase->imageSpaces(piece.image, piece.offset * TilePixels, PlantScanThreshold, piece.flip));
    spaces.addAll(piece.spaces);
  }
  
  data->spaces = spaces.values();
  data->boundBox = RectI::boundBoxOfPoints(data->spaces);
  
  for (auto space : data->spaces) {
    if (space[1] == 0) {
      if (data->ceiling)
        data->roots.push_back({space[0], 1});
      else
        data->roots.push_back({space[0], -1});
    }
  }
}

float PlantAdapter::branchRotation(float xPos, float rotoffset) const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data || !inWorld() || data->windLevel == 0.0f)
    return 0.0f;
  
  float intensity = fabs(data->windLevel);
  return copysign(0.00117f, data->windLevel) * 
         (std::sin(data->windTime + rotoffset + xPos / 10.0f) * intensity - intensity / 300.0f);
}

void PlantAdapter::calcBoundBox() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  RectF boundBox = RectF::boundBoxOfPoints(data->spaces);
  data->metaBoundBox = RectF(boundBox.min() - Vec2F(1, 1), boundBox.max() + Vec2F(2, 2));
  
  auto* bbox = getComponent<BoundingBoxComponent>();
  if (bbox)
    bbox->boundingBox = data->metaBoundBox;
}

void PlantAdapter::readPieces(ByteArray pieces) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data || pieces.empty())
    return;
    
  DataStreamBuffer ds(std::move(pieces));
  ds.readContainer(data->pieces, [](DataStream& ds, PlantPieceData& piece) {
      ds.read(piece.image);
      ds.read(piece.offset[0]);
      ds.read(piece.offset[1]);
      int rotType;
      ds.read(rotType);
      piece.rotationType = static_cast<PlantRotationType>(rotType);
      ds.read(piece.rotationOffset);
      ds.read(piece.structuralSegment);
      int pieceKind;
      ds.read(pieceKind);
      piece.kind = static_cast<PlantPieceKind>(pieceKind);
      ds.read(piece.segmentIdx);
      ds.read(piece.flip);
    });
  data->piecesScanned = false;
  if (inWorld())
    validatePieces();
}

ByteArray PlantAdapter::writePieces() const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return {};
    
  return DataStreamBuffer::serializeContainer(data->pieces, [](DataStream& ds, PlantPieceData const& piece) {
      ds.write(piece.image);
      ds.write(piece.offset[0]);
      ds.write(piece.offset[1]);
      ds.write(static_cast<int>(piece.rotationType));
      ds.write(piece.rotationOffset);
      ds.write(piece.structuralSegment);
      ds.write(static_cast<int>(piece.kind));
      ds.write(piece.segmentIdx);
      ds.write(piece.flip);
    });
}

void PlantAdapter::readPiecesFromJson(Json const& pieces) {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  static EnumMap<PlantRotationType> const RotationTypeNames{
    {PlantRotationType::DontRotate, "dontRotate"},
    {PlantRotationType::RotateBranch, "rotateBranch"},
    {PlantRotationType::RotateLeaves, "rotateLeaves"},
    {PlantRotationType::RotateCrownBranch, "rotateCrownBranch"},
    {PlantRotationType::RotateCrownLeaves, "rotateCrownLeaves"},
  };
  
  data->pieces = jsonToList<PlantPieceData>(pieces, [&](Json const& v) -> PlantPieceData {
      PlantPieceData res;
      res.image = v.getString("image");
      res.offset = jsonToVec2F(v.get("offset"));
      res.rotationType = RotationTypeNames.getLeft(v.getString("rotationType"));
      res.rotationOffset = v.getFloat("rotationOffset");
      res.structuralSegment = v.getBool("structuralSegment");
      res.kind = static_cast<PlantPieceKind>(v.getInt("kind"));
      res.segmentIdx = v.getInt("segmentIdx");
      res.flip = v.getBool("flip");
      return res;
    });
  data->piecesScanned = false;
  if (inWorld())
    validatePieces();
}

Json PlantAdapter::writePiecesToJson() const {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return {};
    
  static EnumMap<PlantRotationType> const RotationTypeNames{
    {PlantRotationType::DontRotate, "dontRotate"},
    {PlantRotationType::RotateBranch, "rotateBranch"},
    {PlantRotationType::RotateLeaves, "rotateLeaves"},
    {PlantRotationType::RotateCrownBranch, "rotateCrownBranch"},
    {PlantRotationType::RotateCrownLeaves, "rotateCrownLeaves"},
  };
  
  return jsonFromList<PlantPieceData>(data->pieces, [&](PlantPieceData const& piece) -> Json {
      return JsonObject{
        {"image", piece.image},
        {"offset", jsonFromVec2F(piece.offset)},
        {"rotationType", RotationTypeNames.getRight(piece.rotationType)},
        {"rotationOffset", piece.rotationOffset},
        {"structuralSegment", piece.structuralSegment},
        {"kind", static_cast<int>(piece.kind)},
        {"segmentIdx", piece.segmentIdx},
        {"flip", piece.flip},
      };
    });
}

void PlantAdapter::validatePieces() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  for (auto& piece : data->pieces)
    piece.imagePath = piece.image;
  if (!data->piecesScanned) {
    scanSpacesAndRoots();
    calcBoundBox();
    data->piecesScanned = true;
  }
}

void PlantAdapter::setupNetStates() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  m_netGroup.addNetElement(&data->tileDamageStatus);
  m_netGroup.addNetElement(&m_piecesNetState);
  m_netGroup.addNetElement(&m_tileDamageXNetState);
  m_netGroup.addNetElement(&m_tileDamageYNetState);
  m_netGroup.addNetElement(&m_tileDamageEventNetState);
  
  m_netGroup.setNeedsStoreCallback(bind(&PlantAdapter::setNetStates, this));
  m_netGroup.setNeedsLoadCallback(bind(&PlantAdapter::getNetStates, this));
}

void PlantAdapter::getNetStates() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  if (m_piecesNetState.pullUpdated()) {
    readPieces(m_piecesNetState.get());
    data->piecesUpdated = true;
  }
  
  data->tileDamageX = m_tileDamageXNetState.get();
  data->tileDamageY = m_tileDamageYNetState.get();
  if (m_tileDamageEventNetState.pullOccurred()) {
    data->tileDamageEvent = true;
    data->tileDamageEventTrigger = true;
  }
}

void PlantAdapter::setNetStates() {
  auto* data = getComponent<PlantDataComponent>();
  if (!data)
    return;
    
  if (data->piecesUpdated) {
    m_piecesNetState.set(writePieces());
    data->piecesUpdated = false;
  }
  m_tileDamageXNetState.set(data->tileDamageX);
  m_tileDamageYNetState.set(data->tileDamageY);
  if (data->tileDamageEventTrigger) {
    data->tileDamageEventTrigger = false;
    m_tileDamageEventNetState.trigger();
  }
}

} // namespace ECS
} // namespace Star
