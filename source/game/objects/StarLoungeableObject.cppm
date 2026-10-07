module;
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"

#include "StarObject.hpp"
import star.drawable;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.anchorable_entity;
import star.entity_rendering_types;
import star.lounging_entities;

export module star.loungeable_object;

export namespace Star {

class LoungeableObject : public Object, public virtual LoungeableEntity {
public:
  LoungeableObject(ObjectConfigConstPtr config, Json const& parameters = Json());

  void render(RenderCallback* renderCallback) override;

  InteractAction interact(InteractRequest const& request) override;

  size_t anchorCount() const override;
  LoungeAnchorConstPtr loungeAnchor(size_t positionIndex) const override;

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
  Maybe<String> m_sitEmote;
  Maybe<String> m_sitDance;
  JsonObject m_sitArmorCosmeticOverrides;
  Maybe<String> m_sitCursorOverride;
};

}
