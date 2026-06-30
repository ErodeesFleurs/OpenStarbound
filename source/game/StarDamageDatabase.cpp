#include "StarDamageDatabase.hpp"
#include "StarAlgorithm.hpp"
#include "StarRandom.hpp"
#include "StarAssets.hpp"

namespace Star {

DamageDatabase::DamageDatabase(AssetsConstPtr assets) {
  assets = requireServiceValueAs<StarException>(std::move(assets), "DamageDatabase", "assets");
  auto elementalConfig = assets->json("/damage/elementaltypes.config");
  for (auto const& [elementalTypeName, elementalTypeConfig] : elementalConfig.iterateObject()) {
    ElementalType type;
    type.resistanceStat = elementalTypeConfig.getString("resistanceStat");
    for (auto const& [hitTypeName, particleName] : elementalTypeConfig.getObject("damageNumberParticles")) {
      type.damageNumberParticles.set(HitTypeNames.getLeft(hitTypeName), particleName.toString());
    }
    m_elementalTypes.set(elementalTypeName, std::move(type));
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
    for (auto const& [materialName, materialEffects] : config.getObject("effects", JsonObject())) {
      TargetMaterial material = materialName;
      kind.effects.set(material, {});
      for (auto const& [hitTypeName, hitEffectConfig] : materialEffects.iterateObject()) {
        DamageEffect dmgEffect = DamageEffect {
          hitEffectConfig.get("sounds", JsonArray()),
          hitEffectConfig.get("particles", JsonArray())
        };
        kind.effects[material].set(HitTypeNames.getLeft(hitTypeName), dmgEffect);
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
