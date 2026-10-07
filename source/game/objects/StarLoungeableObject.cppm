module;
#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarMathCommon.hpp"
#include "StarRandom.hpp"
import star.periodic;
#include "StarInterpolation.hpp"
import star.periodic_function;
#include "StarNetElementSystem.hpp"
#include "StarSet.hpp"
#include "StarLua.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarMaybe.hpp"
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"


#include "StarLuaRoot.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering;
import star.object;
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


namespace Star {

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

export module star.loungeable_object;

export namespace Star {
  using ::Star::LoungeableObject;
}
