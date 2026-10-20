import star.json;
import star.data_stream;
import star.game_types;
import star.random;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.format;
import star.version;

import star.damage_types;
import star.asset_source;
import star.assets;

import star.damage_database;

namespace Star {

DamageDatabase::DamageDatabase(std::shared_ptr<Assets const> assets) {

  auto elementalConfig = assets->json("/damage/elementaltypes.config");
  for (auto p : elementalConfig.iterateObject()) {
    ElementalType type;
    type.resistanceStat = p.second.getString("resistanceStat");
    for (auto particle : p.second.getObject("damageNumberParticles")) {
      type.damageNumberParticles.set(HitTypeNames.getLeft(particle.first), particle.second.toString());
    }
    m_elementalTypes.set(p.first, std::move(type));
  }

  auto& files = assets->scanExtension("damage");
  assets->queueJsons(files);
  for (auto& file : files) {
    auto config = assets->json(file);
    String name = config.getString("kind");
    if (m_damageKinds.contains(name))
      throw StarException(strf("Duplicate damage kind Name {}. configfile {}", name, file));

    DamageKind kind;
    kind.name = name;
    for (auto effect : config.getObject("effects", JsonObject())) {
      TargetMaterial material = effect.first;
      kind.effects.set(material, {});
      for (auto hit : effect.second.toObject()) {
        DamageEffect effect = DamageEffect {
          hit.second.get("sounds", JsonArray()),
          hit.second.get("particles", JsonArray())
        };
        kind.effects[material].set(HitTypeNames.getLeft(hit.first), effect);
      }
    }
    kind.elementalType = config.getString("elementalType", "default");
    if (!m_elementalTypes.contains(kind.elementalType))
      throw StarException(strf("Undefined elemental type {} in damage kind {}", kind.elementalType, name));

    m_damageKinds.set(name, std::move(kind));
  }
}

DamageKind const& DamageDatabase::damageKind(String kind) const {
  if (kind.empty())
    kind = "default";
  else
    kind = kind.toLower();

  if (!m_damageKinds.contains(kind))
    throw StarException(strf("Unknown damage definition with kind '{}'.", kind));

  return m_damageKinds.get(kind);
}

ElementalType const& DamageDatabase::elementalType(String const& name) const {
  if (!m_damageKinds.contains(name))
    throw StarException(strf("Unknown elemental type with name '{}'.", name));

  return m_elementalTypes.get(name);
}

}
