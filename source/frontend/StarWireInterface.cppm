module;
import std;
import star.json;

import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;


import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;
import star.world_geometry;
import star.wiring;

namespace Star {

class WorldClient;
class WorldPainter;
class Player;

class WirePane : public Pane, public WireConnector {
public:
  WirePane(std::shared_ptr<WorldClient> worldClient, std::shared_ptr<Player> player, std::shared_ptr<WorldPainter> worldPainter);
  virtual ~WirePane() {}

  virtual void update(float dt) override;
  virtual bool sendEvent(InputEvent const& event) override;

  virtual SwingResult swing(WorldGeometry const& geometry, Vec2F position, FireMode mode) override;
  virtual bool connecting() override;

  virtual void reset();

protected:
  void renderImpl() override;

private:
  void renderWire(Vec2F from, Vec2F to, Color baseColor);

  std::shared_ptr<WorldClient> m_worldClient;
  std::shared_ptr<Player> m_player;
  std::shared_ptr<WorldPainter> m_worldPainter;
  Vec2I m_mousePos;
  bool m_connecting;
  WireDirection m_sourceDirection;
  WireConnection m_sourceConnector;

  Vec2F m_inSize;
  Vec2F m_outSize;
  Vec2F m_nodeSize;

  float m_beamWidthDev;
  float m_minBeamWidth;
  float m_maxBeamWidth;
  float m_beamTransDev;
  float m_minBeamTrans;
  float m_maxBeamTrans;
  float m_innerBrightnessScale;
  float m_firstStripeThickness;
  float m_secondStripeThickness;
};

}

export module star.wire_interface;

export namespace Star {
  using ::Star::WorldClient;
  using ::Star::WorldPainter;
  using ::Star::Player;
  using ::Star::WirePane;
}
