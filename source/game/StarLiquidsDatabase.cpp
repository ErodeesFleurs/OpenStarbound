#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarEither.hpp"
#include "StarGameTypes.hpp"
#include "StarList.hpp"
#include "StarVariant.hpp"
#include "StarRect.hpp"
#include "StarMultiArray.hpp"
#include "StarMap.hpp"
#include "StarOrderedSet.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarLexicalCast.hpp"
#include "StarOrderedMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.collision_block;
import star.cellular_liquid;
import star.item_descriptor;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.liquids_database;
import star.material_database;


namespace Star {

LiquidSettings::LiquidSettings() : id(EmptyLiquidId) {}

LiquidsDatabase::LiquidsDatabase() {
  auto assets = Root::singleton().assets();
  auto materialDatabase = Root::singleton().materialDatabase();

  auto config = assets->json("/liquids.config");

  auto liquidEngineParameters = config.get("liquidEngineParameters");
  m_liquidEngineParameters.lateralMoveFactor = liquidEngineParameters.getFloat("lateralMoveFactor");
  m_liquidEngineParameters.spreadOverfillUpFactor = liquidEngineParameters.getFloat("spreadOverfillUpFactor");
  m_liquidEngineParameters.spreadOverfillLateralFactor = liquidEngineParameters.getFloat("spreadOverfillLateralFactor");
  m_liquidEngineParameters.spreadOverfillDownFactor = liquidEngineParameters.getFloat("spreadOverfillDownFactor");
  m_liquidEngineParameters.pressureEqualizeFactor = liquidEngineParameters.getFloat("pressureEqualizeFactor");
  m_liquidEngineParameters.pressureMoveFactor = liquidEngineParameters.getFloat("pressureMoveFactor");
  m_liquidEngineParameters.maximumPressureLevelImbalance = liquidEngineParameters.getFloat("maximumPressureLevelImbalance");
  m_liquidEngineParameters.minimumLivenPressureChange = liquidEngineParameters.getFloat("minimumLivenPressureChange");
  m_liquidEngineParameters.minimumLivenLevelChange = liquidEngineParameters.getFloat("minimumLivenLevelChange");
  m_liquidEngineParameters.minimumLiquidLevel = liquidEngineParameters.getFloat("minimumLiquidLevel");
  m_liquidEngineParameters.interactTransformationLevel = liquidEngineParameters.getFloat("interactTransformationLevel");

  m_backgroundDrain = config.getFloat("backgroundDrain");

  m_liquidNames["empty"] = EmptyLiquidId;

  auto& liquids = assets->scanExtension("liquid");
  assets->queueJsons(liquids);

  for (auto& file : liquids) {
    try {
      auto liquidConfig = assets->json(file);

      unsigned id = liquidConfig.getUInt("liquidId");
      if (id != (LiquidId)id)
        throw LiquidException(strf("Liquid id {} does not fall in the valid range, wrapped to {}\n", id, (LiquidId)id));

      auto entry = make_shared<LiquidSettings>();
      entry->id = (LiquidId)id;
      entry->name = liquidConfig.getString("name");
      entry->path = file;
      entry->config = liquidConfig;

      JsonObject descriptions;
      for (auto pair : liquidConfig.iterateObject())
        if (pair.first.endsWith("Description"))
          descriptions[pair.first] = pair.second;
      descriptions["description"] = liquidConfig.getString("description", "");
      entry->descriptions = descriptions;

      entry->tickDelta = liquidConfig.getUInt("tickDelta");
      entry->liquidColor = jsonToVec4B(liquidConfig.get("color"));

      if (liquidConfig.contains("radiantLight")) {
        auto lightLevel = jsonToVec3B(liquidConfig.get("radiantLight"));
        entry->radiantLightLevel = Color::rgb(lightLevel).toRgbF();
      }

      entry->itemDrop = ItemDescriptor(liquidConfig.get("itemDrop", {}));

      entry->statusEffects = liquidConfig.getArray("statusEffects", {});

      for (auto const& interaction : liquidConfig.getArray("interactions", {})) {
        LiquidId liquid = interaction.getUInt("liquid");
        LiquidInteractionResult interactionResult;
        if (auto materialResult = interaction.optString("materialResult"))
          interactionResult = makeLeft<MaterialId>(materialDatabase->materialId(*materialResult));
        else if (auto liquidResult = interaction.optUInt("liquidResult"))
          interactionResult = makeRight<LiquidId>(*liquidResult);
        else
          throw LiquidException::format(
              "Neither resultMaterial or resultLiquid specified in liquid interaction of liquid {}", entry->id);

        entry->interactions[liquid] = interactionResult;
      }

      m_settings.set(entry->id, entry);
      m_liquidNames.add(entry->name, entry->id);
    } catch (StarException const& e) {
      throw LiquidException(strf("Error loading liquid file {}", file), e);
    }
  }
}

LiquidCellEngineParameters LiquidsDatabase::liquidEngineParameters() const {
  return m_liquidEngineParameters;
}

StringList LiquidsDatabase::liquidNames() const {
  return m_liquidNames.keys();
}

List<LiquidSettingsConstPtr> LiquidsDatabase::allLiquidSettings() const {
  return filtered<List<LiquidSettingsConstPtr>>(m_settings, mem_fn(&LiquidSettingsConstPtr::operator bool));
}

LiquidId LiquidsDatabase::liquidId(String const& str) const {
  return m_liquidNames.get(str);
}

String LiquidsDatabase::liquidName(LiquidId liquidId) const {
  if (liquidId == EmptyLiquidId)
    return "empty";
  else if (auto settings = liquidSettings(liquidId))
    return settings->name;
  throw LiquidException::format("invalid liquid id {}", liquidId);
}

String LiquidsDatabase::liquidDescription(LiquidId liquidId, String const& species) const {
  if (liquidId == EmptyLiquidId)
    return String();
  else if (auto settings = liquidSettings(liquidId))
    return settings->descriptions.getString(
        strf("{}Description", species), settings->descriptions.getString("description"));
  throw LiquidException::format("invalid liquid id {}", liquidId);
}

String LiquidsDatabase::liquidDescription(LiquidId liquidId) const {
  if (liquidId == EmptyLiquidId)
    return String();
  else if (auto settings = liquidSettings(liquidId))
    return settings->descriptions.getString("description");
  throw LiquidException::format("invalid liquid id {}", liquidId);
}

Maybe<String> LiquidsDatabase::liquidPath(LiquidId liquidId) const {
  if (liquidId == EmptyLiquidId)
    return {};
  else if (auto settings = liquidSettings(liquidId))
    return settings->path;
  return {};
}

Maybe<Json> LiquidsDatabase::liquidConfig(LiquidId liquidId) const {
  if (liquidId == EmptyLiquidId)
    return {};
  else if (auto settings = liquidSettings(liquidId))
    return settings->config;
  return {};
}

Maybe<LiquidInteractionResult> LiquidsDatabase::interact(LiquidId target, LiquidId other) const {
  if (auto settings = liquidSettings(target))
    return settings->interactions.value(other);
  return {};
}

}
