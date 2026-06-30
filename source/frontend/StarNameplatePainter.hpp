#pragma once

#include "StarAssets.hpp"
#include "StarWorldClient.hpp"
#include "StarWorldCamera.hpp"
#include "StarChatBubbleSeparation.hpp"
#include "StarTextPainter.hpp"

namespace Star {

class GuiContext;
class WorldClient;
using WorldClientPtr = SharedPtr<WorldClient>;
class NameplatePainter;
using NameplatePainterPtr = SharedPtr<NameplatePainter>;

class NameplatePainter {
public:
  struct Services {
    AssetsConstPtr assets;
    GuiContext& guiContext;
  };

  explicit NameplatePainter(Services services);

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

  [[nodiscard]] TextPositioning namePosition(Vec2F bubblePosition) const;
  [[nodiscard]] TextPositioning statusPosition(Vec2F bubblePosition) const;
  [[nodiscard]] RectF determineBoundBox(Vec2F bubblePosition, Nametag const& nametag) const;

  GuiContext& m_guiContext;
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
