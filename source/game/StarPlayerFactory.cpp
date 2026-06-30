#include "StarPlayerFactory.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarRoot.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"
#include "StarRebuilder.hpp"

namespace Star {

PlayerConfig::PlayerConfig(JsonObject const& cfg, IAssetsConstPtr assets)
  : humanoidTiming(cfg.contains("humanoidTiming") ? Humanoid::HumanoidTiming(cfg.value("humanoidTiming")) : Humanoid::HumanoidTiming::sensibleDefaults(std::move(assets))) {
  defaultIdentity = HumanoidIdentity(cfg.value("defaultHumanoidIdentity"));

  for (Json v : cfg.value("defaultItems", JsonArray()).toArray())
    defaultItems.append(ItemDescriptor(v));

  for (Json v : cfg.value("defaultBlueprints", JsonObject()).getArray("tier1", JsonArray()))
    defaultBlueprints.append(ItemDescriptor(v));

  metaBoundBox = jsonToRectF(cfg.get("metaBoundBox"));

  movementParameters = cfg.get("movementParameters");
  zeroGMovementParameters = cfg.get("zeroGMovementParameters");
  statusControllerSettings = cfg.get("statusControllerSettings");

  footstepTiming = cfg.get("footstepTiming").toFloat();
  footstepSensor = jsonToVec2F(cfg.get("footstepSensor"));

  underwaterSensor = jsonToVec2F(cfg.get("underwaterSensor"));
  underwaterMinWaterLevel = cfg.get("underwaterMinWaterLevel").toFloat();

  splashConfig = EntitySplashConfig(cfg.get("splashConfig"));

  companionsConfig = cfg.get("companionsConfig");

  deploymentConfig = cfg.get("deploymentConfig");

  effectsAnimator = cfg.get("effectsAnimator").toString();

  teleportInTime = cfg.get("teleportInTime").toFloat();
  teleportOutTime = cfg.get("teleportOutTime").toFloat();

  deployInTime = cfg.get("deployInTime").toFloat();
  deployOutTime = cfg.get("deployOutTime").toFloat();

  bodyMaterialKind = cfg.get("bodyMaterialKind").toString();

  for (auto& p : cfg.get("genericScriptContexts").optObject().value(JsonObject()))
    genericScriptContexts[p.first] = p.second.toString();
}

PlayerFactory::PlayerFactory(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase)
  : m_assets(std::move(assets)), m_itemDatabase(std::move(itemDatabase)), m_objectDatabase(std::move(objectDatabase)), m_questTemplateDatabase(std::move(questTemplateDatabase)), m_versioningDatabase(std::move(versioningDatabase)), m_rebuilder(make_shared<Rebuilder>(m_assets, "player")) {
  if (!m_assets)
    throw PlayerException("PlayerFactory requires assets service");
  if (!m_objectDatabase)
    throw PlayerException("PlayerFactory requires object database service");
  if (!m_questTemplateDatabase)
    throw PlayerException("PlayerFactory requires quest template database service");
  if (!m_versioningDatabase)
    throw PlayerException("PlayerFactory requires versioning database service");

  m_config = make_shared<PlayerConfig>(m_assets->json("/player.config").toObject(), m_assets);
}

PlayerPtr PlayerFactory::create() const {
  return make_shared<Player>(m_config, Uuid(), m_assets, nullptr, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase);
}

PlayerPtr PlayerFactory::diskLoadPlayer(Json const& diskStore) const {
  PlayerPtr player;
  try {
    player = make_shared<Player>(m_config, diskStore, m_assets, nullptr, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase);
  } catch (std::exception const& e) {
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&](Json const& store) -> String {
      try {
        player = make_shared<Player>(m_config, store, m_assets, nullptr, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase);
      } catch (std::exception const& e) {
        exception = std::current_exception();
        return strf("{}", outputException(e, false));
      }
      return {};
    });

    if (!success)
      std::rethrow_exception(exception);
  }
  return player;
}

PlayerPtr PlayerFactory::netLoadPlayer(ByteArray const& netStore, NetCompatibilityRules rules) const {
  return make_shared<Player>(m_config, netStore, rules, m_assets, nullptr, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase);
}

}
