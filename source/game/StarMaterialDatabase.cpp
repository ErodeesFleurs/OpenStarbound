#include "StarMaterialDatabase.hpp"

#include "StarConfig.hpp"
#include "StarFormat.hpp"
#include "StarJsonExtra.hpp"
#include "StarLogging.hpp"
#include "StarParticleDatabase.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

MaterialDatabase::MaterialDatabase() {
  m_metaModIndex = {
    {"metamod:none", NoModId},
    {"metamod:biome", BiomeModId},
    {"metamod:undergroundbiome", UndergroundBiomeModId}};

  auto assets = Root::singleton().assets();
  auto pdb = Root::singleton().particleDatabase();

  setMetaMaterial(EmptyMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:empty", EmptyMaterialId, CollisionKind::None, false});
  setMetaMaterial(NullMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:null", NullMaterialId, CollisionKind::Block, true});
  setMetaMaterial(StructureMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:structure", StructureMaterialId, CollisionKind::Block, true});
  setMetaMaterial(BiomeMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:biome", BiomeMaterialId, CollisionKind::Block, true});
  setMetaMaterial(Biome1MaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:biome1", Biome1MaterialId, CollisionKind::Block, true});
  setMetaMaterial(Biome2MaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:biome2", Biome2MaterialId, CollisionKind::Block, true});
  setMetaMaterial(Biome3MaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:biome3", Biome3MaterialId, CollisionKind::Block, true});
  setMetaMaterial(Biome4MaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:biome4", Biome4MaterialId, CollisionKind::Block, true});
  setMetaMaterial(Biome5MaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:biome5", Biome5MaterialId, CollisionKind::Block, true});
  setMetaMaterial(BoundaryMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:boundary", BoundaryMaterialId, CollisionKind::Slippery, true});
  setMetaMaterial(ObjectSolidMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:objectsolid", ObjectSolidMaterialId, CollisionKind::Block, true});
  setMetaMaterial(ObjectPlatformMaterialId, MaterialDatabase::MetaMaterialInfo{"metamaterial:objectplatform", ObjectPlatformMaterialId, CollisionKind::Platform, false});

  auto metaMaterialConfig = assets->json("/metamaterials.config");
  for (auto metaMaterial : metaMaterialConfig.iterateArray()) {
    auto matName = "metamaterial:" + metaMaterial.getString("name");
    if (isMaterialName(matName)) {
      Logger::info("Metamaterial '{}' has duplicate material name!", matName);
      continue;
    }

    MaterialId matId = metaMaterial.getUInt("materialId");
    if (isRealMaterial(matId) || matId >= FirstEngineMetaMaterialId) {
      Logger::info("Material id {} for metamaterial '{}' does not fall within the valid range!", matId, matName);
      continue;
    } else if (containsMetaMaterial(matId)) {
      Logger::info("Material id {} for metamaterial '{}' conflicts with another metamaterial id!", matId, matName);
      continue;
    }

    auto matCollisionKind = CollisionKindNames.getLeft(metaMaterial.getString("collisionKind"));

    bool blocksLiquidFlow = metaMaterial.getBool("blocksLiquidFlow", isSolidColliding(matCollisionKind));

    setMetaMaterial(matId, MaterialDatabase::MetaMaterialInfo{matName, matId, matCollisionKind, blocksLiquidFlow});
  }

  auto& materials = assets->scanExtension("material");
  auto& mods = assets->scanExtension("matmod");

  assets->queueJsons(materials);
  assets->queueJsons(mods);

  for (auto& file : materials) {
    try {
      auto matConfig = assets->json(file);

      MaterialInfo material;

      auto materialId = matConfig.getInt("materialId");
      material.id = materialId;

      material.name = matConfig.getString("materialName");
      material.path = file;
      material.config = matConfig;

      material.itemDrop = matConfig.getString("itemDrop", "");

      JsonObject descriptions;
      for (auto entry : matConfig.iterateObject())
        if (entry.first.endsWith("Description"))
          descriptions[entry.first] = entry.second;
      descriptions["description"] = matConfig.getString("description", "");
      descriptions["shortdescription"] = matConfig.getString("shortdescription", "");
      material.descriptions = descriptions;

      material.category = matConfig.getString("category");

      material.particleColor = jsonToColor(matConfig.get("particleColor", JsonArray{0, 0, 0, 255}));
      if (matConfig.contains("miningParticle"))
        material.miningParticle = pdb->config(matConfig.getString("miningParticle"));
      if (matConfig.contains("miningSounds"))
        material.miningSounds = transform<StringList>(
          jsonToStringList(matConfig.get("miningSounds")), [file](auto&& PH1) -> auto { return AssetPath::relativeTo(file, std::forward<decltype(PH1)>(PH1)); });
      if (matConfig.contains("footstepSound"))
        material.footstepSound = AssetPath::relativeTo(file, matConfig.getString("footstepSound"));

      material.tillableMod = matConfig.getInt("tillableMod", NoModId);
      material.soil = matConfig.getBool("soil", false);
      material.falling = matConfig.getBool("falling", false);
      material.cascading = matConfig.getBool("cascading", false);

      if (matConfig.contains("renderTemplate")) {
        auto renderTemplate = assets->fetchJson(matConfig.get("renderTemplate"), file);
        auto renderParameters = matConfig.get("renderParameters");
        material.materialRenderProfile = std::make_shared<MaterialRenderProfile>(parseMaterialRenderProfile(jsonMerge(renderTemplate, renderParameters), file));
      }

      material.damageParameters =
        TileDamageParameters(assets->fetchJson(matConfig.get("damageTable", "/tiles/defaultDamage.config")),
                             matConfig.optFloat("health"),
                             matConfig.optUInt("requiredHarvestLevel"));

      material.collisionKind = CollisionKindNames.getLeft(matConfig.getString("collisionKind", "block"));
      material.foregroundOnly = matConfig.getBool("foregroundOnly", false);
      material.supportsMods = matConfig.getBool("supportsMods", !(material.falling || material.cascading || material.collisionKind != CollisionKind::Block));

      material.blocksLiquidFlow = matConfig.getBool("blocksLiquidFlow", isSolidColliding(material.collisionKind));

      if (material.id != materialId || !isRealMaterial(material.id))
        throw MaterialException(strf("Material id {} does not fall in the valid range\n", materialId));

      if (containsMaterial(material.id))
        throw MaterialException(strf("Duplicate material id {} found for material {}", material.id, material.name));

      if (isMaterialName(material.name))
        throw MaterialException(strf("Duplicate material name '{}' found", material.name));

      setMaterial(material.id, material);

      for (auto liquidInteraction : matConfig.getArray("liquidInteractions", {})) {
        LiquidId liquidId = liquidInteraction.getUInt("liquidId");
        LiquidMaterialInteraction interaction;
        interaction.consumeLiquid = liquidInteraction.getFloat("consumeLiquid", 0.0f);
        interaction.transformTo = liquidInteraction.getUInt("transformMaterialId", NullMaterialId);
        interaction.topOnly = liquidInteraction.getBool("topOnly", false);
        m_liquidMaterialInteractions[{liquidId, material.id}] = interaction;
      }
    } catch (StarException const& e) {
      throw MaterialException(strf("Error loading material file {}", file), e);
    }
  }

  for (auto& file : mods) {
    try {
      auto modConfig = assets->json(file);

      ModInfo mod;

      auto modId = modConfig.getInt("modId");
      mod.id = modId;

      mod.name = modConfig.getString("modName");
      mod.path = file;
      mod.config = modConfig;

      mod.itemDrop = modConfig.getString("itemDrop", "");

      JsonObject descriptions;
      for (auto entry : modConfig.iterateObject())
        if (entry.first.endsWith("Description"))
          descriptions[entry.first] = entry.second;
      descriptions["description"] = modConfig.getString("description", "");
      mod.descriptions = descriptions;

      mod.particleColor = jsonToColor(modConfig.get("particleColor", JsonArray{0, 0, 0, 255}));
      if (modConfig.contains("miningParticle"))
        mod.miningParticle = pdb->config(modConfig.getString("miningParticle"));
      if (modConfig.contains("miningSounds"))
        mod.miningSounds = transform<StringList>(
          jsonToStringList(modConfig.get("miningSounds")), [file](auto&& PH1) -> auto { return AssetPath::relativeTo(file, std::forward<decltype(PH1)>(PH1)); });
      if (modConfig.contains("footstepSound"))
        mod.footstepSound = AssetPath::relativeTo(file, modConfig.getString("footstepSound"));

      mod.tilled = modConfig.getBool("tilled", false);

      mod.breaksWithTile = modConfig.getBool("breaksWithTile", false);

      if (modConfig.contains("renderTemplate")) {
        auto renderTemplate = assets->fetchJson(modConfig.get("renderTemplate"));
        auto renderParameters = modConfig.get("renderParameters");
        mod.modRenderProfile = std::make_shared<MaterialRenderProfile>(parseMaterialRenderProfile(jsonMerge(renderTemplate, renderParameters), file));
      }

      mod.damageParameters =
        TileDamageParameters(assets->fetchJson(modConfig.get("damageTable", "/tiles/defaultDamage.config")),
                             modConfig.optFloat("health"),
                             modConfig.optUInt("harvestLevel"));

      if (modId != mod.id || !isRealMod(mod.id))
        throw MaterialException(strf("Mod id {} does not fall in the valid range\n", mod.id));

      if (containsMod(mod.id))
        throw MaterialException(strf("Duplicate mod id {} found for mod {}", mod.id, mod.name));

      if (m_modIndex.contains(mod.name) || m_metaModIndex.hasLeftValue(mod.name))
        throw MaterialException(strf("Duplicate mod name '{}' found", mod.name));

      setMod(mod.id, mod);
      m_modIndex[mod.name] = mod.id;

      for (auto liquidInteraction : modConfig.getArray("liquidInteractions", {})) {
        LiquidId liquidId = liquidInteraction.getUInt("liquidId");
        LiquidModInteraction interaction;
        interaction.consumeLiquid = liquidInteraction.getFloat("consumeLiquid", 0.0f);
        interaction.transformTo = liquidInteraction.getUInt("transformModId", NoModId);
        interaction.topOnly = liquidInteraction.getBool("topOnly", false);
        m_liquidModInteractions[{liquidId, mod.id}] = interaction;
      }
    } catch (StarException const& e) {
      throw MaterialException(strf("Error loading mod file {}", file), e);
    }
  }

  m_defaultFootstepSound = assets->json("/client.config:defaultFootstepSound").toString();
}

auto MaterialDatabase::materialNames() const -> StringList {
  StringList names = m_materialIndex.keys();
  names.appendAll(m_metaMaterialIndex.keys());
  return names;
}

auto MaterialDatabase::isMetaMaterialName(String const& name) const -> bool {
  return m_metaMaterialIndex.contains(name);
}

auto MaterialDatabase::isMaterialName(String const& name) const -> bool {
  return m_materialIndex.contains(name) || m_metaMaterialIndex.contains(name);
}

auto MaterialDatabase::isValidMaterialId(MaterialId material) const -> bool {
  if (isRealMaterial(material))
    return containsMaterial(material);
  else
    return containsMetaMaterial(material);
}

auto MaterialDatabase::materialId(String const& matName) const -> MaterialId {
  if (auto m = m_metaMaterialIndex.maybe(matName))
    return *m;
  else
    return m_materialIndex.get(matName);
}

auto MaterialDatabase::materialName(MaterialId materialId) const -> String {
  if (isRealMaterial(materialId))
    return getMaterialInfo(materialId)->name;
  else
    return getMetaMaterialInfo(materialId)->name;
}

auto MaterialDatabase::materialPath(MaterialId materialId) const -> std::optional<String> {
  if (isRealMaterial(materialId))
    return getMaterialInfo(materialId)->path;
  else
    return {};
}

auto MaterialDatabase::materialConfig(MaterialId materialId) const -> std::optional<Json> {
  if (isRealMaterial(materialId))
    return getMaterialInfo(materialId)->config;
  else
    return {};
}

auto MaterialDatabase::materialDescription(MaterialId materialNumber, String const& species) const -> String {
  auto material = m_materials[materialNumber];
  return material->descriptions.getString(
    strf("{}Description", species), material->descriptions.getString("description"));
}

auto MaterialDatabase::materialDescription(MaterialId materialNumber) const -> String {
  auto material = m_materials[materialNumber];
  return material->descriptions.getString("description");
}

auto MaterialDatabase::materialShortDescription(MaterialId materialNumber) const -> String {
  auto material = m_materials[materialNumber];
  return material->descriptions.getString("shortdescription");
}

auto MaterialDatabase::materialCategory(MaterialId materialNumber) const -> String {
  auto material = m_materials[materialNumber];
  return material->category;
}

auto MaterialDatabase::modNames() const -> StringList {
  StringList modNames = m_modIndex.keys();
  modNames.appendAll(m_metaModIndex.leftValues());
  return modNames;
}

auto MaterialDatabase::isModName(String const& name) const -> bool {
  return m_modIndex.contains(name);
}

auto MaterialDatabase::isValidModId(ModId mod) const -> bool {
  if (isRealMod(mod))
    return mod < m_mods.size() && (bool)m_mods[mod];
  else
    return m_metaModIndex.hasRightValue(mod);
}

auto MaterialDatabase::modId(String const& modName) const -> ModId {
  if (auto m = m_metaModIndex.maybeRight(modName))
    return *m;
  else
    return m_modIndex.get(modName);
}

auto MaterialDatabase::modName(ModId mod) const -> String const& {
  if (isRealMod(mod))
    return getModInfo(mod)->name;
  else
    return m_metaModIndex.getLeft(mod);
}

auto MaterialDatabase::modPath(ModId mod) const -> std::optional<String> {
  if (isRealMod(mod))
    return getModInfo(mod)->path;
  else
    return {};
}

auto MaterialDatabase::modConfig(ModId mod) const -> std::optional<Json> {
  if (isRealMod(mod))
    return getModInfo(mod)->config;
  else
    return {};
}

auto MaterialDatabase::modDescription(ModId modId, String const& species) const -> String {
  auto mod = m_mods[modId];
  return mod->descriptions.getString(strf("{}Description", species), mod->descriptions.getString("description"));
}

auto MaterialDatabase::modDescription(ModId modId) const -> String {
  auto mod = m_mods[modId];
  return mod->descriptions.getString("description");
}

auto MaterialDatabase::modShortDescription(ModId modId) const -> String {
  auto mod = m_mods[modId];
  return mod->descriptions.getString("shortdescription");
}

auto MaterialDatabase::defaultFootstepSound() const -> String {
  return m_defaultFootstepSound;
}

auto MaterialDatabase::materialDamageParameters(MaterialId materialId) const -> TileDamageParameters {
  if (!isRealMaterial(materialId))
    return {};
  else
    return getMaterialInfo(materialId)->damageParameters;
}

auto MaterialDatabase::modDamageParameters(ModId modId) const -> TileDamageParameters {
  if (!isRealMod(modId))
    return {};
  else
    return getModInfo(modId)->damageParameters;
}

auto MaterialDatabase::modBreaksWithTile(ModId modId) const -> bool {
  if (!isRealMod(modId))
    return {};
  else
    return getModInfo(modId)->breaksWithTile;
}

auto MaterialDatabase::materialCollisionKind(MaterialId materialId) const -> CollisionKind {
  if (isRealMaterial(materialId))
    return getMaterialInfo(materialId)->collisionKind;
  else if (containsMetaMaterial(materialId))
    return getMetaMaterialInfo(materialId)->collisionKind;
  else
    return CollisionKind::Block;
}

auto MaterialDatabase::canPlaceInLayer(MaterialId materialId, TileLayer layer) const -> bool {
  return layer != TileLayer::Background || !getMaterialInfo(materialId)->foregroundOnly;
}

auto MaterialDatabase::materialItemDrop(MaterialId materialId) const -> ItemDescriptor {
  if (isRealMaterial(materialId)) {
    auto matInfo = getMaterialInfo(materialId);
    if (!matInfo->itemDrop.empty())
      return {matInfo->itemDrop, 1, JsonObject()};
  }

  return {};
}

auto MaterialDatabase::modItemDrop(ModId modId) const -> ItemDescriptor {
  if (isRealMod(modId)) {
    auto modInfo = getModInfo(modId);
    if (!modInfo->itemDrop.empty())
      return {modInfo->itemDrop, 1};
  }

  return {};
}

auto MaterialDatabase::materialColorVariants(MaterialId materialId) const -> MaterialColorVariant {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->colorVariants;
  }

  return 0;
}

auto MaterialDatabase::modColorVariants(ModId modId) const -> MaterialColorVariant {
  if (isRealMod(modId)) {
    auto const& modInfo = getModInfo(modId);
    if (modInfo->modRenderProfile)
      return modInfo->modRenderProfile->colorVariants;
  }

  return 0;
}

auto MaterialDatabase::isMultiColor(MaterialId materialId) const -> bool {
  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->materialRenderProfile)
      return matInfo->materialRenderProfile->colorVariants > 0;
  }

  return false;
}

auto MaterialDatabase::miningParticle(MaterialId materialId, ModId modId) const -> Ptr<ParticleConfig> {
  if (isRealMod(modId)) {
    auto const& modInfo = getModInfo(modId);
    if (modInfo->miningParticle)
      return modInfo->miningParticle;
  }

  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (matInfo->miningParticle)
      return matInfo->miningParticle;
  }

  return {};
}

auto MaterialDatabase::miningSound(MaterialId materialId, ModId modId) const -> String {
  if (isRealMod(modId)) {
    auto const& modInfo = getModInfo(modId);
    if (!modInfo->miningSounds.empty())
      return Random::randValueFrom(modInfo->miningSounds);
  }

  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (!matInfo->miningSounds.empty())
      return Random::randValueFrom(matInfo->miningSounds);
  }

  return {};
}

auto MaterialDatabase::footstepSound(MaterialId materialId, ModId modId) const -> String {
  if (isRealMod(modId)) {
    auto const& modInfo = getModInfo(modId);
    if (!modInfo->footstepSound.empty())
      return modInfo->footstepSound;
  }

  if (isRealMaterial(materialId)) {
    auto const& matInfo = getMaterialInfo(materialId);
    if (!matInfo->footstepSound.empty())
      return matInfo->footstepSound;
  }

  return m_defaultFootstepSound;
}

auto MaterialDatabase::materialParticleColor(MaterialId materialId, MaterialHue hueShift) const -> Color {
  auto color = getMaterialInfo(materialId)->particleColor;
  color.setHue(pfmod(color.hue() + materialHueToDegrees(hueShift) / 360.0f, 1.0f));
  return color;
}

auto MaterialDatabase::isTilledMod(ModId modId) const -> bool {
  if (!isRealMod(modId))
    return false;
  return getModInfo(modId)->tilled;
}

auto MaterialDatabase::isSoil(MaterialId materialId) const -> bool {
  if (!isRealMaterial(materialId))
    return false;
  return getMaterialInfo(materialId)->soil;
}

auto MaterialDatabase::tilledModFor(MaterialId materialId) const -> ModId {
  if (!isRealMaterial(materialId))
    return NoModId;
  return getMaterialInfo(materialId)->tillableMod;
}

auto MaterialDatabase::isFallingMaterial(MaterialId materialId) const -> bool {
  if (!isRealMaterial(materialId))
    return false;
  return getMaterialInfo(materialId)->falling;
}

auto MaterialDatabase::isCascadingFallingMaterial(MaterialId materialId) const -> bool {
  if (!isRealMaterial(materialId))
    return false;
  return getMaterialInfo(materialId)->cascading;
}

auto MaterialDatabase::supportsMod(MaterialId materialId, ModId modId) const -> bool {
  if (modId == NoModId)
    return true;
  if (!isRealMaterial(materialId))
    return false;
  if (!isRealMod(modId))
    return false;

  return getMaterialInfo(materialId)->supportsMods;
}

MaterialDatabase::MetaMaterialInfo::MetaMaterialInfo(String name, MaterialId id, CollisionKind collisionKind, bool blocksLiquidFlow)
    : name(std::move(name)), id(id), collisionKind(collisionKind), blocksLiquidFlow(blocksLiquidFlow) {}

MaterialDatabase::MaterialInfo::MaterialInfo() : id(NullMaterialId), tillableMod(NoModId), falling(), cascading() {}

MaterialDatabase::ModInfo::ModInfo() : id(NoModId), tilled(), breaksWithTile() {}

auto MaterialDatabase::metaMaterialIndex(MaterialId materialId) const -> size_t {
  return materialId - FirstMetaMaterialId;
}

auto MaterialDatabase::containsMetaMaterial(MaterialId materialId) const -> bool {
  auto i = metaMaterialIndex(materialId);
  return m_metaMaterials.size() > i && m_metaMaterials[i];
}

void MaterialDatabase::setMetaMaterial(MaterialId materialId, MetaMaterialInfo info) {
  auto i = metaMaterialIndex(materialId);
  if (i >= m_metaMaterials.size())
    m_metaMaterials.resize(i + 1);
  m_metaMaterials[i] = std::make_shared<MetaMaterialInfo>(info);
  m_metaMaterialIndex[info.name] = materialId;
}

auto MaterialDatabase::containsMaterial(MaterialId materialId) const -> bool {
  return m_materials.size() > materialId && m_materials[materialId];
}

void MaterialDatabase::setMaterial(MaterialId materialId, MaterialInfo info) {
  if (materialId >= m_materials.size())
    m_materials.resize(materialId + 1);
  m_materials[materialId] = std::make_shared<MaterialInfo>(info);
  m_materialIndex[info.name] = materialId;
}

auto MaterialDatabase::containsMod(ModId modId) const -> bool {
  return m_mods.size() > modId && m_mods[modId];
}

void MaterialDatabase::setMod(ModId modId, ModInfo info) {
  if (modId >= m_mods.size())
    m_mods.resize(modId + 1);
  m_mods[modId] = std::make_shared<ModInfo>(info);
}

auto MaterialDatabase::getMetaMaterialInfo(MaterialId materialId) const -> std::shared_ptr<MaterialDatabase::MetaMaterialInfo const> const& {
  if (!containsMetaMaterial(materialId))
    throw MaterialException(strf("No such metamaterial id: {}\n", materialId));
  else
    return m_metaMaterials[metaMaterialIndex(materialId)];
}

auto MaterialDatabase::getMaterialInfo(MaterialId materialId) const -> std::shared_ptr<MaterialDatabase::MaterialInfo const> const& {
  if (materialId >= m_materials.size() || !m_materials[materialId])
    throw MaterialException(strf("No such material id: {}\n", materialId));
  else
    return m_materials[materialId];
}

auto MaterialDatabase::getModInfo(ModId modId) const -> std::shared_ptr<MaterialDatabase::ModInfo const> const& {
  if (modId >= m_mods.size() || !m_mods[modId])
    throw MaterialException(strf("No such mod id: {}\n", modId));
  else
    return m_mods[modId];
}

}// namespace Star
