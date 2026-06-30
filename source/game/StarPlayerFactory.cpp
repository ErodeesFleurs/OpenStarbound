#include "StarPlayerFactory.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarRoot.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"
#include "StarRebuilder.hpp"

namespace Star {

PlayerConfig::PlayerConfig(JsonObject const& cfg, AssetsConstPtr assets)
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

  splashConfig = EntitySplashConfig(cfg.get("splashConfig"), assets);

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

PlayerFactory::PlayerFactory(AssetsConstPtr assets, ConfigurationPtr configuration, MaterialDatabaseConstPtr materialDatabase, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase, CodexDatabaseConstPtr codexDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, RadioMessageDatabaseConstPtr radioMessageDatabase, AiDatabaseConstPtr aiDatabase, CollectionDatabaseConstPtr collectionDatabase, SpeciesDatabaseConstPtr speciesDatabase, function<EntityFactoryConstPtr()> entityFactory, LiquidsDatabaseConstPtr liquidsDatabase, TechDatabaseConstPtr techDatabase)
  : m_assets(std::move(assets)),
    m_configuration(std::move(configuration)),
    m_materialDatabase(std::move(materialDatabase)),
    m_itemDatabase(std::move(itemDatabase)),
    m_objectDatabase(std::move(objectDatabase)),
    m_questTemplateDatabase(std::move(questTemplateDatabase)),
    m_versioningDatabase(std::move(versioningDatabase)),
    m_codexDatabase(std::move(codexDatabase)),
    m_danceDatabase(std::move(danceDatabase)),
    m_emoteProcessor(std::move(emoteProcessor)),
    m_radioMessageDatabase(std::move(radioMessageDatabase)),
    m_aiDatabase(std::move(aiDatabase)),
    m_collectionDatabase(std::move(collectionDatabase)),
    m_speciesDatabase(std::move(speciesDatabase)),
    m_entityFactory(std::move(entityFactory)),
    m_liquidsDatabase(std::move(liquidsDatabase)),
    m_techDatabase(std::move(techDatabase)),
    m_rebuilder(make_shared<Rebuilder>(m_assets, "player")) {
  if (!m_assets)
    throw PlayerException("PlayerFactory requires assets service");
  if (!m_configuration)
    throw PlayerException("PlayerFactory requires configuration service");
  if (!m_materialDatabase)
    throw PlayerException("PlayerFactory requires material database service");
  if (!m_itemDatabase)
    throw PlayerException("PlayerFactory requires item database service");
  if (!m_objectDatabase)
    throw PlayerException("PlayerFactory requires object database service");
  if (!m_questTemplateDatabase)
    throw PlayerException("PlayerFactory requires quest template database service");
  if (!m_versioningDatabase)
    throw PlayerException("PlayerFactory requires versioning database service");
  if (!m_codexDatabase)
    throw PlayerException("PlayerFactory requires codex database service");
  if (!m_danceDatabase)
    throw PlayerException("PlayerFactory requires dance database service");
  if (!m_emoteProcessor)
    throw PlayerException("PlayerFactory requires emote processor service");
  if (!m_radioMessageDatabase)
    throw PlayerException("PlayerFactory requires radio message database service");
  if (!m_aiDatabase)
    throw PlayerException("PlayerFactory requires ai database service");
  if (!m_collectionDatabase)
    throw PlayerException("PlayerFactory requires collection database service");
  if (!m_speciesDatabase)
    throw PlayerException("PlayerFactory requires species database service");
  if (!m_entityFactory)
    throw PlayerException("PlayerFactory requires entity factory service");
  if (!m_liquidsDatabase)
    throw PlayerException("PlayerFactory requires liquids database service");
  if (!m_techDatabase)
    throw PlayerException("PlayerFactory requires tech database service");

  m_config = make_shared<PlayerConfig>(m_assets->json("/player.config").toObject(), m_assets);
}

PlayerPtr PlayerFactory::create() const {
  return make_shared<Player>(m_config, Uuid(), m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase);
}

PlayerPtr PlayerFactory::diskLoadPlayer(Json const& diskStore) const {
  PlayerPtr player;
  try {
    player = make_shared<Player>(m_config, diskStore, m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase);
  } catch (std::exception const& e) {
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&](Json const& store) -> String {
      try {
        player = make_shared<Player>(m_config, store, m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase);
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
  return make_shared<Player>(m_config, netStore, rules, m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase);
}

}
