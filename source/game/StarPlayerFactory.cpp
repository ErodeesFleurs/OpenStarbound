#include "StarPlayerFactory.hpp"
#include "StarAlgorithm.hpp"
#include "StarAssets.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarRebuilder.hpp"
#include "StarRoot.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarUtilityLuaBindings.hpp"

namespace Star {

PlayerConfig::PlayerConfig(JsonObject const& cfg, AssetsConstPtr assets)
    : humanoidTiming(cfg.contains("humanoidTiming") ? Humanoid::HumanoidTiming(cfg.value("humanoidTiming")) : Humanoid::HumanoidTiming::sensibleDefaults(requireServiceValueAs<StarException>(std::move(assets), "PlayerConfig", "assets"))) {
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

  for (auto& [contextName, contextPath] : cfg.get("genericScriptContexts").optObject().value(JsonObject()))
    genericScriptContexts[contextName] = contextPath.toString();
}

PlayerFactory::PlayerFactory(AssetsConstPtr assets, ConfigurationPtr configuration, MaterialDatabaseConstPtr materialDatabase, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase, CodexDatabaseConstPtr codexDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, RadioMessageDatabaseConstPtr radioMessageDatabase, AiDatabaseConstPtr aiDatabase, CollectionDatabaseConstPtr collectionDatabase, SpeciesDatabaseConstPtr speciesDatabase, function<EntityFactoryConstPtr()> entityFactory, LiquidsDatabaseConstPtr liquidsDatabase, TechDatabaseConstPtr techDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, LuaRootServices luaRootServices)
    : m_assets(requireServiceValueAs<PlayerException>(std::move(assets), "PlayerFactory", "assets")),
      m_configuration(requireServiceValueAs<PlayerException>(std::move(configuration), "PlayerFactory", "configuration")),
      m_materialDatabase(requireServiceValueAs<PlayerException>(std::move(materialDatabase), "PlayerFactory", "material database")),
      m_itemDatabase(requireServiceValueAs<PlayerException>(std::move(itemDatabase), "PlayerFactory", "item database")),
      m_objectDatabase(requireServiceValueAs<PlayerException>(std::move(objectDatabase), "PlayerFactory", "object database")),
      m_questTemplateDatabase(requireServiceValueAs<PlayerException>(std::move(questTemplateDatabase), "PlayerFactory", "quest template database")),
      m_versioningDatabase(requireServiceValueAs<PlayerException>(std::move(versioningDatabase), "PlayerFactory", "versioning database")),
      m_codexDatabase(requireServiceValueAs<PlayerException>(std::move(codexDatabase), "PlayerFactory", "codex database")),
      m_danceDatabase(requireServiceValueAs<PlayerException>(std::move(danceDatabase), "PlayerFactory", "dance database")),
      m_emoteProcessor(requireServiceValueAs<PlayerException>(std::move(emoteProcessor), "PlayerFactory", "emote processor")),
      m_radioMessageDatabase(requireServiceValueAs<PlayerException>(std::move(radioMessageDatabase), "PlayerFactory", "radio message database")),
      m_aiDatabase(requireServiceValueAs<PlayerException>(std::move(aiDatabase), "PlayerFactory", "ai database")),
      m_collectionDatabase(requireServiceValueAs<PlayerException>(std::move(collectionDatabase), "PlayerFactory", "collection database")),
      m_speciesDatabase(requireServiceValueAs<PlayerException>(std::move(speciesDatabase), "PlayerFactory", "species database")),
      m_entityFactory(requireServiceValueAs<PlayerException>(std::move(entityFactory), "PlayerFactory", "entity factory")),
      m_liquidsDatabase(requireServiceValueAs<PlayerException>(std::move(liquidsDatabase), "PlayerFactory", "liquids database")),
      m_techDatabase(requireServiceValueAs<PlayerException>(std::move(techDatabase), "PlayerFactory", "tech database")),
      m_statusEffectDatabase(requireServiceValueAs<PlayerException>(std::move(statusEffectDatabase), "PlayerFactory", "status effect database")),
      m_particleDatabase(requireServiceValueAs<PlayerException>(std::move(particleDatabase), "PlayerFactory", "particle database")),
      m_imageMetadataDatabase(requireServiceValueAs<PlayerException>(std::move(imageMetadataDatabase), "PlayerFactory", "image metadata database")),
      m_rebuilder(make_shared<Rebuilder>(m_assets, "player", requireLuaRootServices(std::move(luaRootServices), "PlayerFactory"))) {
  m_config = make_shared<PlayerConfig>(m_assets->json("/player.config").toObject(), m_assets);
}

PlayerPtr PlayerFactory::create() const {
  return make_shared<Player>(m_config, Uuid(), m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase, m_statusEffectDatabase, m_particleDatabase, m_imageMetadataDatabase);
}

PlayerPtr PlayerFactory::diskLoadPlayer(Json const& diskStore) const {
  PlayerPtr player;
  try {
    player = make_shared<Player>(m_config, diskStore, m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase, m_statusEffectDatabase, m_particleDatabase, m_imageMetadataDatabase);
  } catch (std::exception const& e) {
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&](Json const& store) -> String {
      try {
        player = make_shared<Player>(m_config, store, m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase, m_statusEffectDatabase, m_particleDatabase, m_imageMetadataDatabase);
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
  return make_shared<Player>(m_config, netStore, rules, m_assets, m_configuration, m_materialDatabase, m_itemDatabase, m_objectDatabase, m_questTemplateDatabase, m_versioningDatabase, m_codexDatabase, m_danceDatabase, m_emoteProcessor, m_radioMessageDatabase, m_aiDatabase, m_collectionDatabase, m_speciesDatabase, m_entityFactory(), m_liquidsDatabase, m_techDatabase, m_statusEffectDatabase, m_particleDatabase, m_imageMetadataDatabase);
}

}// namespace Star
