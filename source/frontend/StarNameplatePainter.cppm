module;
import std;
import star.maybe;
import star.json;

import star.game_types;


import star.rect;
import star.list;
import star.set;


import star.color;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.vector;
import star.text;



import star.world_geometry;
import star.chat_bubble_separation;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.world_camera;

namespace Star {

class WorldClient;

class NameplatePainter {
public:
  NameplatePainter();

  void update(float dt, std::shared_ptr<WorldClient> const& world, WorldCamera const& camera, bool inspectionMode);
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
  using ::Star::NameplatePainter;
}
