#pragma once

#include "StarAssets.hpp"
#include "StarColor.hpp"
#include "StarCollisionBlock.hpp"
#include "StarMaterialRenderProfile.hpp"
#include "StarTileDamage.hpp"
#include "StarItemDescriptor.hpp"

namespace Star {

class ParticleConfig;
using ParticleConfigPtr = SharedPtr<ParticleConfig>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;
class MaterialDatabase;
using MaterialDatabasePtr = SharedPtr<MaterialDatabase>;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;

struct MaterialExceptionTag { static constexpr char const* typeName = "MaterialException"; };
using MaterialException = TypedException<StarException, MaterialExceptionTag>;

struct LiquidMaterialInteraction {
  float consumeLiquid;
  MaterialId transformTo;
  bool topOnly;
};

struct LiquidModInteraction {
  float consumeLiquid;
  ModId transformTo;
  bool topOnly;
};

class MaterialDatabase {
public:
  MaterialDatabase(AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] StringList materialNames() const;
  [[nodiscard]] bool isMetaMaterialName(String const& name) const;
  [[nodiscard]] bool isMaterialName(String const& name) const;
  [[nodiscard]] bool isValidMaterialId(MaterialId material) const;
  [[nodiscard]] MaterialId materialId(String const& materialName) const;
  [[nodiscard]] String materialName(MaterialId materialId) const;
  [[nodiscard]] Maybe<String> materialPath(MaterialId materialId) const;
  [[nodiscard]] Maybe<Json> materialConfig(MaterialId materialId) const;
  [[nodiscard]] String materialDescription(MaterialId materialId, String const& species) const;
  [[nodiscard]] String materialDescription(MaterialId materialId) const;
  [[nodiscard]] String materialShortDescription(MaterialId materialId) const;
  [[nodiscard]] String materialCategory(MaterialId materialId) const;

  [[nodiscard]] StringList modNames() const;
  [[nodiscard]] bool isModName(String const& name) const;
  [[nodiscard]] bool isValidModId(ModId mod) const;
  [[nodiscard]] ModId modId(String const& modName) const;
  [[nodiscard]] String const& modName(ModId modId) const;
  [[nodiscard]] Maybe<String> modPath(ModId modId) const;
  [[nodiscard]] Maybe<Json> modConfig(ModId modId) const;
  [[nodiscard]] String modDescription(ModId modId, String const& species) const;
  [[nodiscard]] String modDescription(ModId modId) const;
  [[nodiscard]] String modShortDescription(ModId modId) const;

  // Will return nullptr if no rendering profile is available
  [[nodiscard]] MaterialRenderProfileConstPtr materialRenderProfile(MaterialId modId) const;
  [[nodiscard]] MaterialRenderProfileConstPtr modRenderProfile(ModId modId) const;

  [[nodiscard]] TileDamageParameters materialDamageParameters(MaterialId materialId) const;
  [[nodiscard]] TileDamageParameters modDamageParameters(ModId modId) const;

  [[nodiscard]] bool modBreaksWithTile(ModId modId) const;

  [[nodiscard]] CollisionKind materialCollisionKind(MaterialId materialId) const;
  [[nodiscard]] bool canPlaceInLayer(MaterialId materialId, TileLayer layer) const;

  // Returned ItemDescriptor may be null
  [[nodiscard]] ItemDescriptor materialItemDrop(MaterialId materialId) const;
  [[nodiscard]] ItemDescriptor modItemDrop(ModId modId) const;

  [[nodiscard]] MaterialColorVariant materialColorVariants(MaterialId materialId) const;
  [[nodiscard]] MaterialColorVariant modColorVariants(ModId modId) const;
  [[nodiscard]] bool isMultiColor(MaterialId materialId) const;
  [[nodiscard]] bool foregroundLightTransparent(MaterialId materialId) const;
  [[nodiscard]] bool backgroundLightTransparent(MaterialId materialId) const;
  [[nodiscard]] bool occludesBehind(MaterialId materialId) const;

  [[nodiscard]] ParticleConfigPtr miningParticle(MaterialId materialId, ModId modId = NoModId) const;
  [[nodiscard]] String miningSound(MaterialId materialId, ModId modId = NoModId) const;
  [[nodiscard]] String footstepSound(MaterialId materialId, ModId modId = NoModId) const;
  [[nodiscard]] String defaultFootstepSound() const;

  [[nodiscard]] Color materialParticleColor(MaterialId materialId, MaterialHue hueShift) const;
  [[nodiscard]] Vec3F radiantLight(MaterialId materialId, ModId modId) const;

  [[nodiscard]] bool supportsMod(MaterialId materialId, ModId modId) const;
  [[nodiscard]] ModId tilledModFor(MaterialId materialId) const;
  [[nodiscard]] bool isTilledMod(ModId modId) const;

  [[nodiscard]] bool isSoil(MaterialId materialId) const;
  [[nodiscard]] bool isFallingMaterial(MaterialId materialId) const;
  [[nodiscard]] bool isCascadingFallingMaterial(MaterialId materialId) const;
  [[nodiscard]] bool blocksLiquidFlow(MaterialId materialId) const;

  // Returns the amount of liquid to consume, and optionally the material / mod
  // to transform to (may be NullMaterialId / NullModId)
  [[nodiscard]] Maybe<LiquidMaterialInteraction> liquidMaterialInteraction(LiquidId liquid, MaterialId materialId) const;
  [[nodiscard]] Maybe<LiquidModInteraction> liquidModInteraction(LiquidId liquid, ModId modId) const;

private:
  struct MetaMaterialInfo {
    MetaMaterialInfo(String name, MaterialId id, CollisionKind collisionKind, bool blocksLiquidFlow);

    String name;
    MaterialId id;
    CollisionKind collisionKind;
    bool blocksLiquidFlow;
  };

  struct MaterialInfo {
    MaterialInfo() = default;

    String name;
    MaterialId id = NullMaterialId;
    String path;
    Json config;

    String itemDrop;
    Json descriptions;
    String category;
    Color particleColor;
    ParticleConfigPtr miningParticle;
    StringList miningSounds;
    String footstepSound;
    ModId tillableMod = NoModId;
    CollisionKind collisionKind = CollisionKind::None;
    bool foregroundOnly = false;
    bool supportsMods = false;
    bool soil = false;
    bool falling{};
    bool cascading{};
    bool blocksLiquidFlow = false;

    shared_ptr<MaterialRenderProfile const> materialRenderProfile;

    TileDamageParameters damageParameters;
  };

  struct ModInfo {
    ModInfo() = default;

    String name;
    ModId id = NoModId;
    String path;
    Json config;

    String itemDrop;
    Json descriptions;
    Color particleColor;
    ParticleConfigPtr miningParticle;
    StringList miningSounds;
    String footstepSound;
    bool tilled{};
    bool breaksWithTile{};

    shared_ptr<MaterialRenderProfile const> modRenderProfile;

    TileDamageParameters damageParameters;
  };

  [[nodiscard]] size_t metaMaterialIndex(MaterialId materialId) const;
  [[nodiscard]] bool containsMetaMaterial(MaterialId materialId) const;
  void setMetaMaterial(MaterialId materialId, MetaMaterialInfo info);

  [[nodiscard]] bool containsMaterial(MaterialId materialId) const;
  void setMaterial(MaterialId materialId, MaterialInfo info);

  [[nodiscard]] bool containsMod(ModId modId) const;
  void setMod(ModId modId, ModInfo info);

  [[nodiscard]] shared_ptr<MetaMaterialInfo const> const& getMetaMaterialInfo(MaterialId materialId) const;
  [[nodiscard]] shared_ptr<MaterialInfo const> const& getMaterialInfo(MaterialId materialId) const;
  [[nodiscard]] shared_ptr<ModInfo const> const& getModInfo(ModId modId) const;

  List<shared_ptr<MetaMaterialInfo const>> m_metaMaterials;
  StringMap<MaterialId> m_metaMaterialIndex;

  List<shared_ptr<MaterialInfo const>> m_materials;
  StringMap<MaterialId> m_materialIndex;

  List<shared_ptr<ModInfo const>> m_mods;
  StringMap<ModId> m_modIndex;
  BiMap<String, ModId> m_metaModIndex;

  String m_defaultFootstepSound;

  HashMap<pair<LiquidId, MaterialId>, LiquidMaterialInteraction> m_liquidMaterialInteractions;
  HashMap<pair<LiquidId, ModId>, LiquidModInteraction> m_liquidModInteractions;
};

[[nodiscard]] inline MaterialRenderProfileConstPtr MaterialDatabase::materialRenderProfile(MaterialId materialId) const {
  if (materialId < m_materials.size()) {
    if (auto const& mat = m_materials[materialId])
      return mat->materialRenderProfile;
  }
  
  return {};
}

[[nodiscard]] inline MaterialRenderProfileConstPtr MaterialDatabase::modRenderProfile(ModId modId) const {
  if (modId < m_mods.size()) {
    if (auto const& mod = m_mods[modId])
      return mod->modRenderProfile;
  }
  
  return {};
}

[[nodiscard]] inline bool MaterialDatabase::foregroundLightTransparent(MaterialId materialId) const {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->foregroundLightTransparent;
  }

  if (materialId == StructureMaterialId)
    return false;

  return true;
}

[[nodiscard]] inline bool MaterialDatabase::backgroundLightTransparent(MaterialId materialId) const {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->backgroundLightTransparent;
  }

  if (materialId == StructureMaterialId)
    return false;

  return true;
}

[[nodiscard]] inline bool MaterialDatabase::occludesBehind(MaterialId materialId) const {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->occludesBehind;
  }

  return false;
}

[[nodiscard]] inline Vec3F MaterialDatabase::radiantLight(MaterialId materialId, ModId modId) const {
  Vec3F radiantLight;
  if (materialId < m_materials.size()) {
    auto const& mat = m_materials[materialId];
    if (mat && mat->materialRenderProfile)
      radiantLight += mat->materialRenderProfile->radiantLight;
  }
  if (modId < m_mods.size()) {
    auto const& mod = m_mods[modId];
    if (mod && mod->modRenderProfile)
      radiantLight += mod->modRenderProfile->radiantLight;
  }
  return radiantLight;
}

[[nodiscard]] inline bool MaterialDatabase::blocksLiquidFlow(MaterialId materialId) const {
  if (isRealMaterial(materialId))
    return getMaterialInfo(materialId)->blocksLiquidFlow;
  else
    return getMetaMaterialInfo(materialId)->blocksLiquidFlow;

}

[[nodiscard]] inline Maybe<LiquidMaterialInteraction> MaterialDatabase::liquidMaterialInteraction(
    LiquidId liquid, MaterialId materialId) const {
  return m_liquidMaterialInteractions.maybe({liquid, materialId});
}

[[nodiscard]] inline Maybe<LiquidModInteraction> MaterialDatabase::liquidModInteraction(LiquidId liquid, ModId modId) const {
  return m_liquidModInteractions.maybe({liquid, modId});
}
}
