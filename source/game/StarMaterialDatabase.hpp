#pragma once

#include "StarCollisionBlock.hpp"
#include "StarColor.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLiquidTypes.hpp"
#include "StarMaterialRenderProfile.hpp"
#include "StarTileDamage.hpp"

import std;

namespace Star {

class ParticleConfig;

using MaterialException = ExceptionDerived<"MaterialException">;

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
  MaterialDatabase();

  [[nodiscard]] auto materialNames() const -> StringList;
  [[nodiscard]] auto isMetaMaterialName(String const& name) const -> bool;
  [[nodiscard]] auto isMaterialName(String const& name) const -> bool;
  [[nodiscard]] auto isValidMaterialId(MaterialId material) const -> bool;
  [[nodiscard]] auto materialId(String const& materialName) const -> MaterialId;
  [[nodiscard]] auto materialName(MaterialId materialId) const -> String;
  [[nodiscard]] auto materialPath(MaterialId materialId) const -> std::optional<String>;
  [[nodiscard]] auto materialConfig(MaterialId materialId) const -> std::optional<Json>;
  [[nodiscard]] auto materialDescription(MaterialId materialId, String const& species) const -> String;
  [[nodiscard]] auto materialDescription(MaterialId materialId) const -> String;
  [[nodiscard]] auto materialShortDescription(MaterialId materialId) const -> String;
  [[nodiscard]] auto materialCategory(MaterialId materialId) const -> String;

  [[nodiscard]] auto modNames() const -> StringList;
  [[nodiscard]] auto isModName(String const& name) const -> bool;
  [[nodiscard]] auto isValidModId(ModId mod) const -> bool;
  [[nodiscard]] auto modId(String const& modName) const -> ModId;
  [[nodiscard]] auto modName(ModId modId) const -> String const&;
  [[nodiscard]] auto modPath(ModId modId) const -> std::optional<String>;
  [[nodiscard]] auto modConfig(ModId modId) const -> std::optional<Json>;
  [[nodiscard]] auto modDescription(ModId modId, String const& species) const -> String;
  [[nodiscard]] auto modDescription(ModId modId) const -> String;
  [[nodiscard]] auto modShortDescription(ModId modId) const -> String;

  // Will return nullptr if no rendering profile is available
  [[nodiscard]] auto materialRenderProfile(MaterialId modId) const -> ConstPtr<MaterialRenderProfile>;
  [[nodiscard]] auto modRenderProfile(ModId modId) const -> ConstPtr<MaterialRenderProfile>;

  [[nodiscard]] auto materialDamageParameters(MaterialId materialId) const -> TileDamageParameters;
  [[nodiscard]] auto modDamageParameters(ModId modId) const -> TileDamageParameters;

  [[nodiscard]] auto modBreaksWithTile(ModId modId) const -> bool;

  [[nodiscard]] auto materialCollisionKind(MaterialId materialId) const -> CollisionKind;
  [[nodiscard]] auto canPlaceInLayer(MaterialId materialId, TileLayer layer) const -> bool;

  // Returned ItemDescriptor may be null
  [[nodiscard]] auto materialItemDrop(MaterialId materialId) const -> ItemDescriptor;
  [[nodiscard]] auto modItemDrop(ModId modId) const -> ItemDescriptor;

  [[nodiscard]] auto materialColorVariants(MaterialId materialId) const -> MaterialColorVariant;
  [[nodiscard]] auto modColorVariants(ModId modId) const -> MaterialColorVariant;
  [[nodiscard]] auto isMultiColor(MaterialId materialId) const -> bool;
  [[nodiscard]] auto foregroundLightTransparent(MaterialId materialId) const -> bool;
  [[nodiscard]] auto backgroundLightTransparent(MaterialId materialId) const -> bool;
  [[nodiscard]] auto occludesBehind(MaterialId materialId) const -> bool;

  [[nodiscard]] auto miningParticle(MaterialId materialId, ModId modId = NoModId) const -> Ptr<ParticleConfig>;
  [[nodiscard]] auto miningSound(MaterialId materialId, ModId modId = NoModId) const -> String;
  [[nodiscard]] auto footstepSound(MaterialId materialId, ModId modId = NoModId) const -> String;
  [[nodiscard]] auto defaultFootstepSound() const -> String;

  [[nodiscard]] auto materialParticleColor(MaterialId materialId, MaterialHue hueShift) const -> Color;
  [[nodiscard]] auto radiantLight(MaterialId materialId, ModId modId) const -> Vec3F;

  [[nodiscard]] auto supportsMod(MaterialId materialId, ModId modId) const -> bool;
  [[nodiscard]] auto tilledModFor(MaterialId materialId) const -> ModId;
  [[nodiscard]] auto isTilledMod(ModId modId) const -> bool;

  [[nodiscard]] auto isSoil(MaterialId materialId) const -> bool;
  [[nodiscard]] auto isFallingMaterial(MaterialId materialId) const -> bool;
  [[nodiscard]] auto isCascadingFallingMaterial(MaterialId materialId) const -> bool;
  [[nodiscard]] auto blocksLiquidFlow(MaterialId materialId) const -> bool;

  // Returns the amount of liquid to consume, and optionally the material / mod
  // to transform to (may be NullMaterialId / NullModId)
  [[nodiscard]] auto liquidMaterialInteraction(LiquidId liquid, MaterialId materialId) const -> std::optional<LiquidMaterialInteraction>;
  [[nodiscard]] auto liquidModInteraction(LiquidId liquid, ModId modId) const -> std::optional<LiquidModInteraction>;

private:
  struct MetaMaterialInfo {
    MetaMaterialInfo(String name, MaterialId id, CollisionKind collisionKind, bool blocksLiquidFlow);

    String name;
    MaterialId id;
    CollisionKind collisionKind;
    bool blocksLiquidFlow;
  };

  struct MaterialInfo {
    MaterialInfo();

    String name;
    MaterialId id;
    String path;
    Json config;

    String itemDrop;
    Json descriptions;
    String category;
    Color particleColor;
    Ptr<ParticleConfig> miningParticle;
    StringList miningSounds;
    String footstepSound;
    ModId tillableMod;
    CollisionKind collisionKind;
    bool foregroundOnly;
    bool supportsMods;
    bool soil;
    bool falling;
    bool cascading;
    bool blocksLiquidFlow;

    std::shared_ptr<MaterialRenderProfile const> materialRenderProfile;

    TileDamageParameters damageParameters;
  };

  struct ModInfo {
    ModInfo();

    String name;
    ModId id;
    String path;
    Json config;

    String itemDrop;
    Json descriptions;
    Color particleColor;
    Ptr<ParticleConfig> miningParticle;
    StringList miningSounds;
    String footstepSound;
    bool tilled;
    bool breaksWithTile;

    std::shared_ptr<MaterialRenderProfile const> modRenderProfile;

    TileDamageParameters damageParameters;
  };

  [[nodiscard]] auto metaMaterialIndex(MaterialId materialId) const -> std::size_t;
  [[nodiscard]] auto containsMetaMaterial(MaterialId materialId) const -> bool;
  void setMetaMaterial(MaterialId materialId, MetaMaterialInfo info);

  [[nodiscard]] auto containsMaterial(MaterialId materialId) const -> bool;
  void setMaterial(MaterialId materialId, MaterialInfo info);

  [[nodiscard]] auto containsMod(ModId modId) const -> bool;
  void setMod(ModId modId, ModInfo info);

  [[nodiscard]] auto getMetaMaterialInfo(MaterialId materialId) const -> std::shared_ptr<MetaMaterialInfo const> const&;
  [[nodiscard]] auto getMaterialInfo(MaterialId materialId) const -> std::shared_ptr<MaterialInfo const> const&;
  [[nodiscard]] auto getModInfo(ModId modId) const -> std::shared_ptr<ModInfo const> const&;

  List<std::shared_ptr<MetaMaterialInfo const>> m_metaMaterials;
  StringMap<MaterialId> m_metaMaterialIndex;

  List<std::shared_ptr<MaterialInfo const>> m_materials;
  StringMap<MaterialId> m_materialIndex;

  List<std::shared_ptr<ModInfo const>> m_mods;
  StringMap<ModId> m_modIndex;
  BiMap<String, ModId> m_metaModIndex;

  String m_defaultFootstepSound;

  HashMap<std::pair<LiquidId, MaterialId>, LiquidMaterialInteraction> m_liquidMaterialInteractions;
  HashMap<std::pair<LiquidId, ModId>, LiquidModInteraction> m_liquidModInteractions;
};

inline auto MaterialDatabase::materialRenderProfile(MaterialId materialId) const -> ConstPtr<MaterialRenderProfile> {
  if (materialId < m_materials.size()) {
    if (auto const& mat = m_materials[materialId])
      return mat->materialRenderProfile;
  }

  return {};
}

inline auto MaterialDatabase::modRenderProfile(ModId modId) const -> ConstPtr<MaterialRenderProfile> {
  if (modId < m_mods.size()) {
    if (auto const& mod = m_mods[modId])
      return mod->modRenderProfile;
  }

  return {};
}

inline auto MaterialDatabase::foregroundLightTransparent(MaterialId materialId) const -> bool {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->foregroundLightTransparent;
  }

  if (materialId == StructureMaterialId)
    return false;

  return true;
}

inline auto MaterialDatabase::backgroundLightTransparent(MaterialId materialId) const -> bool {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->backgroundLightTransparent;
  }

  if (materialId == StructureMaterialId)
    return false;

  return true;
}

inline auto MaterialDatabase::occludesBehind(MaterialId materialId) const -> bool {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->occludesBehind;
  }

  return false;
}

inline auto MaterialDatabase::radiantLight(MaterialId materialId, ModId modId) const -> Vec3F {
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

inline auto MaterialDatabase::blocksLiquidFlow(MaterialId materialId) const -> bool {
  if (isRealMaterial(materialId))
    return getMaterialInfo(materialId)->blocksLiquidFlow;
  else
    return getMetaMaterialInfo(materialId)->blocksLiquidFlow;
}

inline auto MaterialDatabase::liquidMaterialInteraction(
  LiquidId liquid, MaterialId materialId) const -> std::optional<LiquidMaterialInteraction> {
  return m_liquidMaterialInteractions.maybe({liquid, materialId});
}

inline auto MaterialDatabase::liquidModInteraction(LiquidId liquid, ModId modId) const -> std::optional<LiquidModInteraction> {
  return m_liquidModInteractions.maybe({liquid, modId});
}
}// namespace Star
