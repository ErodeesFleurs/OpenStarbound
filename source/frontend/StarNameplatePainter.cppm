module;

#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"


#include "StarRect.hpp"
#include "StarList.hpp"


#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"



import star.world_geometry;
import star.chat_bubble_separation;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.world_camera;

namespace Star {

STAR_CLASS(WorldClient);
STAR_CLASS(NameplatePainter);

class NameplatePainter {
public:
  NameplatePainter();

  void update(float dt, WorldClientPtr const& world, WorldCamera const& camera, bool inspectionMode);
  void render();

private:
  struct Nametag {
    String name;
    Maybe<String> statusText;
    Vec3B color;
    float opacity;
    EntityId entityId;
  };

  TextPositioning namePosition(Vec2F bubblePosition) const;
  TextPositioning statusPosition(Vec2F bubblePosition) const;
  RectF determineBoundBox(Vec2F bubblePosition, Nametag const& nametag) const;

  bool m_showMasterNames;
  float m_opacityRate;
  float m_inspectOpacityRate;
  Vec2F m_offset;
  Vec2F m_statusOffset;
  TextStyle m_textStyle;
  TextStyle m_statusTextStyle;
  float m_opacityBoost;

  WorldCamera m_camera;

  Set<EntityId> m_entitiesWithNametags;
  BubbleSeparator<Nametag> m_nametags;
};

}

export module star.nameplate_painter;

export namespace Star {
  using ::Star::WorldClient;
  using ::Star::WorldClientPtr;
  using ::Star::WorldClientConstPtr;
  using ::Star::WorldClientWeakPtr;
  using ::Star::WorldClientConstWeakPtr;
  using ::Star::WorldClientUPtr;
  using ::Star::WorldClientConstUPtr;
  using ::Star::NameplatePainter;
  using ::Star::NameplatePainterPtr;
  using ::Star::NameplatePainterConstPtr;
  using ::Star::NameplatePainterWeakPtr;
  using ::Star::NameplatePainterConstWeakPtr;
  using ::Star::NameplatePainterUPtr;
  using ::Star::NameplatePainterConstUPtr;
}
