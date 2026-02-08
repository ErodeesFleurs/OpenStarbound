#pragma once

#include "StarConfig.hpp"
#include "StarLoungingEntities.hpp"
#include "StarObject.hpp"

import std;

namespace Star {

class LoungeableObject : public Object, public virtual LoungeableEntity {
public:
  LoungeableObject(ConstPtr<ObjectConfig> config, Json const& parameters = Json());

  void render(RenderCallback* renderCallback) override;

  auto interact(InteractRequest const& request) -> InteractAction override;

  auto anchorCount() const -> size_t override;
  auto loungeAnchor(size_t positionIndex) const -> ConstPtr<LoungeAnchor> override;

protected:
  void setOrientationIndex(size_t orientationIndex) override;

private:
  List<Vec2F> m_sitPositions;
  bool m_sitFlipDirection;
  LoungeOrientation m_sitOrientation;
  float m_sitAngle;
  String m_sitCoverImage;
  bool m_flipImages;
  List<PersistentStatusEffect> m_sitStatusEffects;
  StringSet m_sitEffectEmitters;
  std::optional<String> m_sitEmote;
  std::optional<String> m_sitDance;
  JsonObject m_sitArmorCosmeticOverrides;
  std::optional<String> m_sitCursorOverride;
};

}// namespace Star
