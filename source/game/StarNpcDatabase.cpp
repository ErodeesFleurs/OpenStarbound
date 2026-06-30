#include "StarNpcDatabase.hpp"
#include "StarArmors.hpp"
#include "StarEncode.hpp"
#include "StarItemDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarNameGenerator.hpp"
#include "StarNpc.hpp"
#include "StarRandom.hpp"
#include "StarRebuilder.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarStoredFunctions.hpp"
#include "StarUtilityLuaBindings.hpp"

namespace Star {

NpcDatabase::NpcDatabase(AssetsConstPtr assets,
                         ItemDatabaseConstPtr itemDatabase,
                         ObjectDatabaseConstPtr objectDatabase,
                         SpeciesDatabaseConstPtr speciesDatabase,
                         PatternedNameGeneratorConstPtr nameGenerator,
                         FunctionDatabaseConstPtr functionDatabase,
                         DanceDatabaseConstPtr danceDatabase,
                         EmoteProcessorConstPtr emoteProcessor,
                         VersioningDatabaseConstPtr versioningDatabase,
                         LiquidsDatabaseConstPtr liquidsDatabase,
                         StatusEffectDatabaseConstPtr statusEffectDatabase,
                         ParticleDatabaseConstPtr particleDatabase,
                         ImageMetadataDatabaseConstPtr imageMetadataDatabase,
                         LuaRootServices luaRootServices)
    : m_rebuilder(make_shared<Rebuilder>(assets, "npc", std::move(luaRootServices))),
      m_assets(std::move(assets)),
      m_itemDatabase(std::move(itemDatabase)),
      m_objectDatabase(std::move(objectDatabase)),
      m_speciesDatabase(std::move(speciesDatabase)),
      m_nameGenerator(std::move(nameGenerator)),
      m_functionDatabase(std::move(functionDatabase)),
      m_danceDatabase(std::move(danceDatabase)),
      m_emoteProcessor(std::move(emoteProcessor)),
      m_versioningDatabase(std::move(versioningDatabase)),
      m_liquidsDatabase(std::move(liquidsDatabase)),
      m_statusEffectDatabase(std::move(statusEffectDatabase)),
      m_particleDatabase(std::move(particleDatabase)),
      m_imageMetadataDatabase(std::move(imageMetadataDatabase)) {
  if (!m_assets)
    throw NpcException("NpcDatabase requires assets service");
  if (!m_itemDatabase)
    throw NpcException("NpcDatabase requires item database service");
  if (!m_objectDatabase)
    throw NpcException("NpcDatabase requires object database service");
  if (!m_speciesDatabase)
    throw NpcException("NpcDatabase requires species database service");
  if (!m_nameGenerator)
    throw NpcException("NpcDatabase requires name generator service");
  if (!m_functionDatabase)
    throw NpcException("NpcDatabase requires function database service");
  if (!m_versioningDatabase)
    throw NpcException("NpcDatabase requires versioning database service");
  if (!m_liquidsDatabase)
    throw NpcException("NpcDatabase requires liquids database service");
  if (!m_statusEffectDatabase)
    throw NpcException("NpcDatabase requires status effect database service");
  if (!m_particleDatabase)
    throw NpcException("NpcDatabase requires particle database service");
  if (!m_imageMetadataDatabase)
    throw NpcException("NpcDatabase requires image metadata database service");

  auto& files = m_assets->scanExtension("npctype");
  m_assets->queueJsons(files);
  for (auto& file : files) {
    try {
      auto config = m_assets->json(file);
      String typeName = config.getString("type");

      if (m_npcTypes.contains(typeName))
        throw NpcException(strf("Repeat npc type name '{}'", typeName));

      m_npcTypes[typeName] = config;

    } catch (StarException const& e) {
      throw NpcException(strf("Error loading npc type '{}'", file), e);
    }
  }
}

NpcVariant NpcDatabase::generateNpcVariant(String const& species, String const& typeName, float level) const {
  return generateNpcVariant(species, typeName, level, Random::randu64(), {});
}

NpcVariant NpcDatabase::generateNpcVariant(
  String const& species, String const& typeName, float level, uint64_t seed, Json const& overrides) const {
  NpcVariant variant;
  variant.species = species;
  variant.typeName = typeName;
  variant.seed = seed;
  variant.overrides = overrides;

  RandomSource randSource(seed);

  auto config = buildConfig(typeName, overrides);

  variant.description = config.optString("description");

  auto levelVariance = jsonToVec2F(config.getArray("levelVariance", {0, 0}));
  variant.level = max(randSource.randf(level + levelVariance[0], level + levelVariance[1]), 0.0f);

  variant.scripts = jsonToStringList(config.get("scripts"));
  variant.initialScriptDelta = config.getUInt("initialScriptDelta", 5);
  variant.scriptConfig = config.get("scriptConfig");

  auto speciesDefinition = m_speciesDatabase->species(species);
  auto result = m_speciesDatabase->generateHumanoid(species, seed);
  HumanoidIdentity identity = result.identity;

  variant.humanoidParameters = jsonMerge(result.humanoidParameters, config.getObject("humanoidParameters", JsonObject())).toObject();

  if (config.contains("npcname"))
    identity.name = config.getString("npcname");
  else if (config.contains("nameGen")) {
    identity.name = m_nameGenerator->generateName(jsonToStringList(config.get("nameGen"))[static_cast<int>(identity.gender)], randSource);
  }
  // we're going to kinda end up doing this twice just to make sure it ends up generating with the right personality array
  // its dumb that personality is the only identity value that isn't in the customization screen but comes from the humanoid config
  // therefore we have to do this little dance here for the npcs that get unique humanoid configs, which is silly since if they
  // got a unique humanoid config they probably would have had a predefined personality anyway, but whatever
  if (config.contains("identity"))
    identity = HumanoidIdentity(jsonMerge(identity.toJson(), config.get("identity")));

  variant.uniqueHumanoidConfig = config.contains("humanoidConfig");
  if (variant.uniqueHumanoidConfig) {
    variant.humanoidConfig = m_assets->json(config.getString("humanoidConfig"));
    auto usedHumanoidConfig = m_speciesDatabase->humanoidConfig(identity, variant.humanoidParameters, variant.humanoidConfig);
    // this only needs to be done if the npc has a unique humanoid config, otherwise the output from generateHumanoid should be fine
    identity.personality = parsePersonalityArray(randSource.randFrom(usedHumanoidConfig.getArray("personalities")));
  } else {
    variant.humanoidConfig = speciesDefinition->humanoidConfig();
  }

  // and where it ususally merges the identity
  if (config.contains("identity"))
    identity = HumanoidIdentity(jsonMerge(identity.toJson(), config.get("identity")));

  variant.humanoidIdentity = identity;

  variant.movementParameters = config.get("movementParameters", {});
  variant.statusControllerSettings = config.get("statusControllerSettings");

  float powerMultiplierModifier = m_functionDatabase->function("npcLevelPowerMultiplierModifier")->evaluate(variant.level);
  float protectionMultiplier = m_functionDatabase->function("npcLevelProtectionMultiplier")->evaluate(variant.level);
  float maxHealthMultiplier = m_functionDatabase->function("npcLevelHealthMultiplier")->evaluate(variant.level);
  float maxEnergyMultiplier = m_functionDatabase->function("npcLevelEnergyMultiplier")->evaluate(variant.level);

  variant.innateStatusEffects = config.get("innateStatusEffects", JsonArray()).toArray().transformed(jsonToPersistentStatusEffect);
  variant.innateStatusEffects.append(StatModifier(StatValueModifier{"powerMultiplier", powerMultiplierModifier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"protection", protectionMultiplier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"maxHealth", maxHealthMultiplier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"maxEnergy", maxEnergyMultiplier}));

  variant.touchDamageConfig = config.get("touchDamage", {});

  auto itemsConfig = config.get("items", {});
  if (!itemsConfig.isNull()) {
    auto speciesItemsConfig = itemsConfig.get("override", {});
    if (speciesItemsConfig.isNull())
      speciesItemsConfig = itemsConfig.get(species, {});
    if (speciesItemsConfig.isNull())
      speciesItemsConfig = itemsConfig.get("default", {});

    if (!speciesItemsConfig.isNull()) {
      Json highestLevelItemsConfig;
      for (auto levelItemsConfig : speciesItemsConfig.toArray()) {
        if (variant.level >= levelItemsConfig.getFloat(0)) {
          highestLevelItemsConfig = levelItemsConfig.get(1);
        }
      }

      if (!highestLevelItemsConfig.isNull()) {
        int randomColorIndex = -1;
        bool matchColorIndices = config.getBool("matchColorIndices", false);

        for (auto itemSlotConfig : randSource.randFrom(highestLevelItemsConfig.toArray()).toObject()) {
          ItemDescriptor item = ItemDescriptor(randSource.randFrom(itemSlotConfig.second.toArray()));

          // Randomize color index if colorIndex is an array
          if (item.parameters().contains("colorIndex")) {
            auto colorIndex = item.parameters().get("colorIndex");
            if (colorIndex.isType(Json::Type::Array)) {
              if (!matchColorIndices || randomColorIndex == -1)
                randomColorIndex = randSource.randFrom(colorIndex.toArray()).toInt();

              item = item.applyParameters({{"colorIndex", randomColorIndex}});
            }
          }

          variant.items[itemSlotConfig.first] = std::move(item);
        }
      }
    }
  }

  variant.disableWornArmor = config.getBool("disableWornArmor", true);

  variant.dropPools = jsonToStringList(config.get("dropPools", JsonArray{}));

  variant.persistent = config.getBool("persistent", false);
  variant.keepAlive = config.getBool("keepAlive", false);

  variant.damageTeam = config.getUInt("damageTeam", 0);
  variant.damageTeamType = TeamTypeNames.getLeft(config.getString("damageTeamType", "enemy"));

  variant.nametagColor = jsonToVec3B(config.get("nametagColor", JsonArray{255, 255, 255}));

  variant.splashConfig = EntitySplashConfig(config.get("splashConfig"), m_assets);

  return variant;
}

ByteArray NpcDatabase::writeNpcVariant(NpcVariant const& variant, NetCompatibilityRules rules) const {
  DataStreamBuffer ds;
  ds.setStreamCompatibilityVersion(rules);

  ds.write(variant.species);
  ds.write(variant.typeName);
  ds.write(variant.level);
  ds.write(variant.seed);
  ds.write(variant.overrides);

  ds.write(variant.initialScriptDelta);
  ds.write(variant.humanoidIdentity);
  if (rules.version() >= 11) {
    ds.write(variant.humanoidParameters);
    ds.write(variant.description);
  }

  ds.writeMapContainer(variant.items);

  ds.write(variant.persistent);
  ds.write(variant.keepAlive);
  ds.write(variant.damageTeam);
  ds.write(variant.damageTeamType);

  return ds.data();
}

NpcVariant NpcDatabase::readNpcVariant(ByteArray const& data, NetCompatibilityRules rules) const {
  DataStreamBuffer ds(data);
  ds.setStreamCompatibilityVersion(rules);

  NpcVariant variant;

  ds.read(variant.species);
  ds.read(variant.typeName);
  ds.read(variant.level);
  ds.read(variant.seed);
  ds.read(variant.overrides);

  auto config = buildConfig(variant.typeName, variant.overrides);

  variant.scripts = jsonToStringList(config.get("scripts"));
  variant.scriptConfig = config.get("scriptConfig");

  ds.read(variant.initialScriptDelta);
  ds.read(variant.humanoidIdentity);
  if (rules.version() >= 11) {
    ds.read(variant.humanoidParameters);
    ds.read(variant.description);
  } else {
    variant.humanoidParameters = config.getObject("humanoidParameters", JsonObject());
    variant.description = config.optString("description");
  }

  auto speciesDefinition = m_speciesDatabase->species(variant.species);
  variant.uniqueHumanoidConfig = config.contains("humanoidConfig");
  if (variant.uniqueHumanoidConfig)
    variant.humanoidConfig = m_assets->json(config.getString("humanoidConfig"));
  else
    variant.humanoidConfig = speciesDefinition->humanoidConfig();

  variant.movementParameters = config.get("movementParameters", {});
  variant.statusControllerSettings = config.get("statusControllerSettings");

  float powerMultiplierModifier =
    m_functionDatabase->function("npcLevelPowerMultiplierModifier")->evaluate(variant.level);
  float protectionMultiplier = m_functionDatabase->function("npcLevelProtectionMultiplier")->evaluate(variant.level);
  float maxHealthMultiplier = m_functionDatabase->function("npcLevelHealthMultiplier")->evaluate(variant.level);
  float maxEnergyMultiplier = m_functionDatabase->function("npcLevelEnergyMultiplier")->evaluate(variant.level);

  variant.innateStatusEffects =
    config.get("innateStatusEffects", JsonArray()).toArray().transformed(jsonToPersistentStatusEffect);
  variant.innateStatusEffects.append(StatModifier(StatValueModifier{"powerMultiplier", powerMultiplierModifier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"protection", protectionMultiplier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"maxHealth", maxHealthMultiplier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"maxEnergy", maxEnergyMultiplier}));

  variant.touchDamageConfig = config.get("touchDamage", {});

  ds.readMapContainer(variant.items);

  variant.disableWornArmor = config.getBool("disableWornArmor", true);
  variant.dropPools = jsonToStringList(config.get("dropPools", JsonArray{}));

  ds.read(variant.persistent);
  ds.read(variant.keepAlive);
  ds.read(variant.damageTeam);
  ds.read(variant.damageTeamType);

  variant.nametagColor = jsonToVec3B(config.get("nametagColor", JsonArray{255, 255, 255}));

  variant.splashConfig = EntitySplashConfig(config.get("splashConfig"), m_assets);

  return variant;
}

Json NpcDatabase::writeNpcVariantToJson(NpcVariant const& variant) const {
  JsonObject store{
    {"species", variant.species},
    {"typeName", variant.typeName},
    {"level", variant.level},
    {"seed", variant.seed},
    {"overrides", variant.overrides},
    {"initialScriptDelta", variant.initialScriptDelta},
    {"humanoidIdentity", variant.humanoidIdentity.toJson()},
    {"items", jsonFromMapV<StringMap<ItemDescriptor>>(variant.items, [this](ItemDescriptor const& item) {
       return item.diskStore(m_versioningDatabase);
     })},
    {"persistent", variant.persistent},
    {"keepAlive", variant.keepAlive},
    {"damageTeam", variant.damageTeam},
    {"damageTeamType", TeamTypeNames.getRight(variant.damageTeamType)},
    {"humanoidParameters", variant.humanoidParameters}};
  if (variant.description.isValid())
    store.set("description", variant.description.value());
  return store;
}

NpcVariant NpcDatabase::readNpcVariantFromJson(Json const& data) const {
  NpcVariant variant;

  variant.species = data.getString("species");
  variant.typeName = data.getString("typeName");
  variant.level = data.getFloat("level");
  variant.seed = data.getUInt("seed");
  variant.overrides = data.get("overrides");

  auto config = buildConfig(variant.typeName, variant.overrides);

  variant.description = data.optString("description");

  variant.scripts = jsonToStringList(config.get("scripts"));
  variant.scriptConfig = config.get("scriptConfig");

  variant.initialScriptDelta = data.getInt("initialScriptDelta");
  variant.humanoidIdentity = HumanoidIdentity(data.get("humanoidIdentity"));
  variant.humanoidParameters = data.getObject("humanoidParameters", JsonObject());

  auto speciesDefinition = m_speciesDatabase->species(variant.species);
  variant.uniqueHumanoidConfig = config.contains("humanoidConfig");
  if (variant.uniqueHumanoidConfig)
    variant.humanoidConfig = m_assets->json(config.getString("humanoidConfig"));
  else
    variant.humanoidConfig = speciesDefinition->humanoidConfig();

  variant.movementParameters = config.get("movementParameters", {});
  variant.statusControllerSettings = config.get("statusControllerSettings", {});

  float powerMultiplierModifier =
    m_functionDatabase->function("npcLevelPowerMultiplierModifier")->evaluate(variant.level);
  float protectionMultiplier = m_functionDatabase->function("npcLevelProtectionMultiplier")->evaluate(variant.level);
  float maxHealthMultiplier = m_functionDatabase->function("npcLevelHealthMultiplier")->evaluate(variant.level);
  float maxEnergyMultiplier = m_functionDatabase->function("npcLevelEnergyMultiplier")->evaluate(variant.level);

  variant.innateStatusEffects =
    config.get("innateStatusEffects", JsonArray()).toArray().transformed(jsonToPersistentStatusEffect);
  variant.innateStatusEffects.append(StatModifier(StatValueModifier{"powerMultiplier", powerMultiplierModifier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"protection", protectionMultiplier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"maxHealth", maxHealthMultiplier}));
  variant.innateStatusEffects.append(StatModifier(StatBaseMultiplier{"maxEnergy", maxEnergyMultiplier}));

  variant.touchDamageConfig = config.get("touchDamage", {});

  variant.items = jsonToMapV<StringMap<ItemDescriptor>>(data.get("items"), [this](Json const& item) {
    return ItemDescriptor::loadStore(item, m_versioningDatabase);
  });

  variant.disableWornArmor = config.getBool("disableWornArmor", true);
  variant.dropPools = jsonToStringList(config.get("dropPools", JsonArray{}));

  variant.persistent = data.getBool("persistent");
  variant.keepAlive = data.getBool("keepAlive");
  variant.damageTeam = data.getUInt("damageTeam");
  variant.damageTeamType = TeamTypeNames.getLeft(data.getString("damageTeamType"));

  variant.nametagColor = jsonToVec3B(config.get("nametagColor", JsonArray{255, 255, 255}));

  variant.splashConfig = EntitySplashConfig(config.get("splashConfig"), m_assets);

  return variant;
}

NpcPtr NpcDatabase::createNpc(NpcVariant const& npcVariant) const {
  return make_shared<Npc>(m_assets, NpcDatabaseConstPtr(shared_from_this()), m_imageMetadataDatabase, m_speciesDatabase, m_danceDatabase, m_emoteProcessor, npcVariant, m_itemDatabase, m_objectDatabase, m_liquidsDatabase, m_statusEffectDatabase, m_particleDatabase);
}

NpcPtr NpcDatabase::diskLoadNpc(Json const& diskStore) const {
  NpcPtr npc;
  auto self = NpcDatabaseConstPtr(shared_from_this());
  try {
    NpcVariant npcVariant = readNpcVariantFromJson(diskStore.get("npcVariant"));
    npc = make_shared<Npc>(m_assets, self, m_imageMetadataDatabase, m_speciesDatabase, m_danceDatabase, m_emoteProcessor, npcVariant, diskStore, m_itemDatabase, m_objectDatabase, m_liquidsDatabase, m_statusEffectDatabase, m_particleDatabase);
  } catch (std::exception const& e) {
    auto exception = std::current_exception();
    bool success = m_rebuilder->rebuild(diskStore, strf("{}", outputException(e, false)), [&, self](Json const& store) -> String {
      try {
        NpcVariant npcVariant = readNpcVariantFromJson(store.get("npcVariant"));
        npc = make_shared<Npc>(m_assets, self, m_imageMetadataDatabase, m_speciesDatabase, m_danceDatabase, m_emoteProcessor, npcVariant, store, m_itemDatabase, m_objectDatabase, m_liquidsDatabase, m_statusEffectDatabase, m_particleDatabase);
      } catch (std::exception const& e) {
        exception = std::current_exception();
        return strf("{}", outputException(e, false));
      }
      return {};
    });

    if (!success)
      std::rethrow_exception(exception);
  }
  return npc;
}

NpcPtr NpcDatabase::netLoadNpc(ByteArray const& netStore, NetCompatibilityRules rules) const {
  return make_shared<Npc>(m_assets, NpcDatabaseConstPtr(shared_from_this()), m_imageMetadataDatabase, m_speciesDatabase, m_danceDatabase, m_emoteProcessor, readNpcVariant(netStore, rules), m_itemDatabase, m_objectDatabase, m_liquidsDatabase, m_statusEffectDatabase, m_particleDatabase);
}

List<Drawable> NpcDatabase::npcPortrait(NpcVariant const& npcVariant, PortraitMode mode) const {
  Humanoid humanoid(npcVariant.humanoidIdentity, npcVariant.humanoidParameters, npcVariant.uniqueHumanoidConfig ? npcVariant.humanoidConfig : Json(), m_assets, m_imageMetadataDatabase, m_speciesDatabase, m_danceDatabase, m_particleDatabase);

  auto items = StringMap<ItemDescriptor, CaseInsensitiveStringHash, CaseInsensitiveStringCompare>::from(npcVariant.items);

  auto makeItem = [this, &npcVariant](ItemDescriptor itemDescriptor) -> ItemPtr {
    return m_itemDatabase->item(itemDescriptor, npcVariant.level, npcVariant.seed);
  };

  ArmorWearer armor(m_itemDatabase);
  for (auto item : npcVariant.items) {
    if (auto equipmentSlot = EquipmentSlotNames.maybeLeft(item.first)) {
      armor.setItem(static_cast<uint8_t>(*equipmentSlot), as<ArmorItem>(makeItem(ItemDescriptor(item.second))));
    }
  }

  armor.setupHumanoid(humanoid, false);

  return humanoid.renderPortrait(mode);
}

Json NpcDatabase::buildConfig(String const& typeName, Json const& overrides) const {
  auto const& baseConfig = m_npcTypes.get(typeName);
  auto config = mergeConfigValues(baseConfig, overrides);

  String baseTypeName = baseConfig.getString("baseType", "");
  if (baseTypeName.empty()) {
    return config;
  } else {
    return buildConfig(baseTypeName, config);
  }
}

Json NpcDatabase::mergeConfigValues(Json const& base, Json const& merger) const {
  if (base.type() == Json::Type::Object && merger.type() == Json::Type::Object) {
    auto map = base.toObject();
    for (auto const& entry : merger.iterateObject()) {
      if (!map.insert(entry.first, entry.second).second) {
        map[entry.first] = mergeConfigValues(map[entry.first], entry.second);
      }
    }
    return map;
  } else if (merger.isNull()) {
    return base;
  } else {
    return merger;
  }
}

}// namespace Star
