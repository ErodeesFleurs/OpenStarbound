module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


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

STAR_CLASS(WorldClient);
STAR_CLASS(WorldPainter);
STAR_CLASS(Player);
STAR_CLASS(WirePane);

class WirePane : public Pane, public WireConnector {
public:
  WirePane(WorldClientPtr worldClient, PlayerPtr player, WorldPainterPtr worldPainter);
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

  WorldClientPtr m_worldClient;
  PlayerPtr m_player;
  WorldPainterPtr m_worldPainter;
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
  using ::Star::WorldClientPtr;
  using ::Star::WorldClientConstPtr;
  using ::Star::WorldClientWeakPtr;
  using ::Star::WorldClientConstWeakPtr;
  using ::Star::WorldClientUPtr;
  using ::Star::WorldClientConstUPtr;
  using ::Star::WorldPainter;
  using ::Star::WorldPainterPtr;
  using ::Star::WorldPainterConstPtr;
  using ::Star::WorldPainterWeakPtr;
  using ::Star::WorldPainterConstWeakPtr;
  using ::Star::WorldPainterUPtr;
  using ::Star::WorldPainterConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::WirePane;
  using ::Star::WirePanePtr;
  using ::Star::WirePaneConstPtr;
  using ::Star::WirePaneWeakPtr;
  using ::Star::WirePaneConstWeakPtr;
  using ::Star::WirePaneUPtr;
  using ::Star::WirePaneConstUPtr;
}
