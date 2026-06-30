#pragma once

#include "StarPane.hpp"
#include "StarBiMap.hpp"
#include "StarGameTimers.hpp"

namespace Star {

class PaneManager;
using PaneManagerPtr = SharedPtr<PaneManager>;

enum class PaneLayer {
  Tooltip,
  ModalWindow,
  Window,
  Hud,
  World
};
extern EnumMap<PaneLayer> const PaneLayerNames;

class PaneManager {
public:
  using DismissCallback = function<void(observer_ptr<Pane>)>;

  explicit PaneManager(GuiContext& context);

  void displayPane(PaneLayer paneLayer, UniquePtr<Pane> pane, DismissCallback onDismiss = {});
  void displayPane(PaneLayer paneLayer, Pane& pane, DismissCallback onDismiss = {});

  [[nodiscard]] bool isDisplayed(observer_ptr<Pane> pane) const;

  void dismissPane(observer_ptr<Pane> pane);
  void dismissAllPanes(Set<PaneLayer> const& paneLayers);
  void dismissAllPanes();

  [[nodiscard]] observer_ptr<Pane> topPane(Set<PaneLayer> const& paneLayers) const;
  [[nodiscard]] observer_ptr<Pane> topPane() const;

  void bringToTop(observer_ptr<Pane> pane);
  void bringPaneAdjacent(observer_ptr<Pane> anchor, observer_ptr<Pane> adjacent, int gap);

  [[nodiscard]] observer_ptr<Pane> getPaneAt(Set<PaneLayer> const& paneLayers, Vec2I const& position) const;
  [[nodiscard]] observer_ptr<Pane> getPaneAt(Vec2I const& position) const;
  [[nodiscard]] List<observer_ptr<Pane>> getAllPanes();

  void setBackgroundWidget(UniquePtr<Widget> bg);

  void dismissWhere(function<bool(observer_ptr<Pane>)> func);

  [[nodiscard]] observer_ptr<Pane> keyboardCapturedPane() const;
  [[nodiscard]] Widget* keyboardCapturedWidget() const;
  [[nodiscard]] bool keyboardCapturedForTextInput() const;

  [[nodiscard]] bool sendInputEvent(InputEvent const& event);

  void render();
  void update(float dt);

private:
  struct DisplayEntry {
    observer_ptr<Pane> pane;
    UniquePtr<Pane> ownedPane;
    DismissCallback onDismiss;
  };

  [[nodiscard]] Vec2I windowSize() const;
  [[nodiscard]] Vec2I calculatePaneOffset(observer_ptr<Pane> pane) const;
  [[nodiscard]] Vec2I calculateNewInterfacePosition(observer_ptr<Pane> pane, float interfaceScaleRatio) const;
  [[nodiscard]] bool dismiss(observer_ptr<Pane> pane);

  GuiContext& m_context;
  float m_prevInterfaceScale;

  Map<PaneLayer, List<DisplayEntry>> m_displayedPanes;

  UniquePtr<Widget> m_backgroundWidget;

  float m_tooltipMouseoverRadius;
  Vec2I m_tooltipMouseOffset;
  GameTimer m_tooltipShowTimer;
  Vec2I m_tooltipLastMousePos;
  Vec2I m_tooltipInitialPosition;
  UniquePtr<Pane> m_activeTooltip;
  observer_ptr<Pane> m_tooltipParentPane;
};

}

