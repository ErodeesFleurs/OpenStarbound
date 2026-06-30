#include "StarPaneManager.hpp"
#include "StarGameTypes.hpp"
#include "StarJsonExtra.hpp"
#include "StarAssets.hpp"

namespace Star {

EnumMap<PaneLayer> const PaneLayerNames{
  {PaneLayer::Tooltip, "Tooltip"},
  {PaneLayer::ModalWindow, "ModalWindow"},
  {PaneLayer::Window, "Window"},
  {PaneLayer::Hud, "Hud"},
  {PaneLayer::World, "World"}
};

PaneManager::PaneManager(GuiContext& context)
  : m_context(context), m_prevInterfaceScale(1) {
  auto const& assets = m_context.assets();
  m_tooltipMouseoverRadius = assets->json("/panes.config:tooltipMouseoverRadius").toFloat();
  m_tooltipMouseOffset = jsonToVec2I(assets->json("/panes.config:tooltipMouseoverOffset"));
  m_tooltipShowTimer = GameTimer(assets->json("/panes.config:tooltipMouseoverTime").toFloat());
}

void PaneManager::displayPane(PaneLayer paneLayer, UniquePtr<Pane> pane, DismissCallback onDismiss) {
  auto* rawPane = pane.get();
  if (isDisplayed(observer_ptr<Pane>(rawPane)))
    throw GuiException("Pane displayed twice in PaneManager::displayPane");

  if (!rawPane->hasDisplayed() && rawPane->anchor() == PaneAnchor::None)
    rawPane->setPosition(Vec2I((windowSize() - rawPane->size()) / 2) + rawPane->centerOffset());

  m_displayedPanes[paneLayer].insertAt(0,
      DisplayEntry{observer_ptr<Pane>(rawPane), std::move(pane), std::move(onDismiss)});
  rawPane->displayed();
}

void PaneManager::displayPane(PaneLayer paneLayer, Pane& pane, DismissCallback onDismiss) {
  if (isDisplayed(observer_ptr<Pane>(&pane)))
    throw GuiException("Pane displayed twice in PaneManager::displayPane");

  if (!pane.hasDisplayed() && pane.anchor() == PaneAnchor::None)
    pane.setPosition(Vec2I((windowSize() - pane.size()) / 2) + pane.centerOffset());

  m_displayedPanes[paneLayer].insertAt(0,
      DisplayEntry{observer_ptr<Pane>(&pane), {}, std::move(onDismiss)});
  pane.displayed();
}

bool PaneManager::isDisplayed(observer_ptr<Pane> pane) const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    for (auto const& entry : entries) {
      if (entry.pane == pane)
        return true;
    }
  }
  return false;
}

void PaneManager::dismissPane(observer_ptr<Pane> pane) {
  if (!dismiss(pane))
    throw GuiException("No such pane in PaneManager::dismissPane");
}

void PaneManager::dismissAllPanes(Set<PaneLayer> const& paneLayers) {
  for (auto const& paneLayer : paneLayers) {
    List<observer_ptr<Pane>> toDismiss;
    for (auto const& entry : m_displayedPanes[paneLayer])
      toDismiss.append(entry.pane);
    for (auto const& pane : toDismiss)
      dismiss(pane);
  }
}

void PaneManager::dismissAllPanes() {
  for (auto& [paneLayer, entries] : m_displayedPanes) {
    List<observer_ptr<Pane>> toDismiss;
    for (auto const& entry : entries)
      toDismiss.append(entry.pane);
    for (auto const& pane : toDismiss)
      dismiss(pane);
  }
}

observer_ptr<Pane> PaneManager::topPane(Set<PaneLayer> const& paneLayers) const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    if (paneLayers.contains(paneLayer) && !entries.empty())
      return entries.first().pane;
  }
  return {};
}

observer_ptr<Pane> PaneManager::topPane() const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    if (!entries.empty())
      return entries.first().pane;
  }
  return {};
}

void PaneManager::bringToTop(observer_ptr<Pane> pane) {
  for (auto& [paneLayer, entries] : m_displayedPanes) {
    for (auto i = 0; i < entries.size(); ++i) {
      if (entries[i].pane == pane) {
        auto entry = std::move(entries[i]);
        entries.eraseAt(i);
        entries.insertAt(0, std::move(entry));
        return;
      }
    }
  }
  throw GuiException("Pane was not displayed in PaneManager::bringToTop");
}

void PaneManager::bringPaneAdjacent(observer_ptr<Pane> anchor, observer_ptr<Pane> adjacent, int gap) {
  Vec2I centerAdjacent = anchor->position() + (anchor->size() / 2) - (adjacent->size() / 2);
  centerAdjacent = centerAdjacent.piecewiseClamp(Vec2I(), windowSize() - adjacent->size());

  if (anchor->position()[0] + anchor->size()[0] + gap + adjacent->size()[0] <= windowSize()[0])
    adjacent->setPosition(Vec2I(anchor->position()[0] + anchor->size()[0] + gap, centerAdjacent[1]));
  else if (anchor->position()[0] - gap - adjacent->size()[0] >= 0)
    adjacent->setPosition(Vec2I(anchor->position()[0] - gap - adjacent->size()[0], centerAdjacent[1]));
  else if (anchor->position()[1] + anchor->size()[1] + gap + adjacent->size()[1] <= windowSize()[1])
    adjacent->setPosition(Vec2I(centerAdjacent[0], anchor->position()[1] + anchor->size()[1] + gap));
  else if (anchor->position()[1] - gap - adjacent->size()[1] >= 0)
    adjacent->setPosition(Vec2I(centerAdjacent[0], anchor->position()[1] - gap - adjacent->size()[1]));
  else
    adjacent->setPosition(centerAdjacent);

  bringToTop(adjacent);
}

observer_ptr<Pane> PaneManager::getPaneAt(Set<PaneLayer> const& paneLayers, Vec2I const& position) const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    if (!paneLayers.contains(paneLayer))
      continue;

    for (auto const& entry : entries) {
      if (entry.pane->inWindow(position) && entry.pane->active())
        return entry.pane;
    }
  }
  return {};
}

observer_ptr<Pane> PaneManager::getPaneAt(Vec2I const& position) const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    for (auto const& entry : entries) {
      if (entry.pane.get() != m_activeTooltip.get()
        && entry.pane->inWindow(position)
        && entry.pane->active())
        return entry.pane;
    }
  }
  return {};
}

List<observer_ptr<Pane>> PaneManager::getAllPanes() {
  List<observer_ptr<Pane>> list;
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    for (auto const& entry : entries) {
      if (entry.pane.get() != m_activeTooltip.get() && entry.pane->active())
        list.append(entry.pane);
    }
  }
  return list;
}

void PaneManager::setBackgroundWidget(UniquePtr<Widget> bg) {
  m_backgroundWidget = std::move(bg);
}

void PaneManager::dismissWhere(function<bool(observer_ptr<Pane>)> func) {
  if (!func)
    return;

  for (auto& [paneLayer, entries] : m_displayedPanes) {
    eraseWhere(entries, [&](auto& entry) {
      if (func(entry.pane)) {
        entry.pane->dismissed();
        if (entry.onDismiss)
          entry.onDismiss(entry.pane);
        return true;
      }
      return false;
    });
  }
}

observer_ptr<Pane> PaneManager::keyboardCapturedPane() const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    for (auto const& entry : entries) {
      if (entry.pane->keyboardCapturer())
        return entry.pane;
    }
  }
  return {};
}

Widget* PaneManager::keyboardCapturedWidget() const {
  for (auto const& [paneLayer, entries] : m_displayedPanes) {
    for (auto const& entry : entries) {
      if (auto capturer = entry.pane->keyboardCapturer())
        return capturer;
    }
  }
  return nullptr;
}

bool PaneManager::keyboardCapturedForTextInput() const {
  if (auto widget = keyboardCapturedWidget())
    return widget->keyboardCaptureMode() == KeyboardCaptureMode::TextInput;
  return false;
}

bool PaneManager::sendInputEvent(InputEvent const& event) {
  if (event.is<MouseMoveEvent>()) {
    m_tooltipLastMousePos = *m_context.mousePosition(event);

    for (auto const& [paneLayer, entries] : m_displayedPanes) {
      for (auto const& entry : entries) {
        if (entry.pane->dragActive()) {
          entry.pane->drag(*m_context.mousePosition(event));
          return true;
        }
      }
    }
  }

  if (event.is<MouseButtonDownEvent>()) {
    m_tooltipShowTimer.reset();
    if (m_activeTooltip) {
      dismiss(observer_ptr<Pane>(m_activeTooltip.get()));
      m_activeTooltip.reset();
      m_tooltipParentPane = {};
      m_tooltipShowTimer.reset();
    }
  }

  if (event.is<MouseButtonUpEvent>()) {
    for (auto const& [paneLayer, entries] : m_displayedPanes) {
      for (auto const& entry : entries) {
        if (entry.pane->dragActive()) {
          entry.pane->setDragActive(false, {});
          return true;
        }
      }
    }
  }

  if (event.is<KeyDownEvent>() && m_context.actions(event).contains(InterfaceAction::GuiClose)) {
    if (auto top = topPane({PaneLayer::ModalWindow, PaneLayer::Window})) {
      dismiss(top);
      return true;
    }
  }

  auto keyCapturePane = keyboardCapturedPane();
  if (keyCapturePane && (event.is<KeyDownEvent>() || event.is<KeyUpEvent>() || event.is<TextInputEvent>()))
    return keyCapturePane->sendEvent(event);

  bool foundModal = false;
  for (auto& [paneLayer, entries] : m_displayedPanes) {
    List<observer_ptr<Pane>> panesCopy;
    for (auto const& entry : entries)
      panesCopy.append(entry.pane);
    for (auto const& pane : panesCopy) {
      if (pane->sendEvent(event)) {
        if (event.is<MouseButtonDownEvent>())
          bringToTop(pane);
        return true;
      }
      if (paneLayer == PaneLayer::ModalWindow) {
        foundModal = true;
        break;
      }
    }
    if (foundModal)
      break;
  }

  return false;
}

void PaneManager::render() {
  if (m_backgroundWidget) {
    auto size = m_backgroundWidget->size();
    m_backgroundWidget->setPosition(Vec2I((windowSize()[0] - size[0]) / 2, (windowSize()[1] - size[1]) / 2));
    m_backgroundWidget->render(RectI(Vec2I(), windowSize()));
  }

  for (auto const& [paneLayer, entries] : reverseIterate(m_displayedPanes)) {
    for (auto const& entry : reverseIterate(entries)) {
      auto* pane = entry.pane.get();
      if (pane->active()) {
        if (m_prevInterfaceScale != m_context.interfaceScale())
          pane->setPosition(
              calculateNewInterfacePosition(entry.pane, static_cast<float>(m_context.interfaceScale()) / m_prevInterfaceScale));

        pane->setDrawingOffset(calculatePaneOffset(entry.pane));
        pane->render(RectI(Vec2I(), windowSize()));
      }
    }
  }

  m_context.resetInterfaceScissorRect();
  m_prevInterfaceScale = m_context.interfaceScale();
}

void PaneManager::update(float dt) {
  auto newTooltipParentPane = getPaneAt(m_tooltipLastMousePos);

  bool updateTooltip = m_tooltipShowTimer.tick(dt) || (m_activeTooltip && (
    vmag(m_tooltipInitialPosition - m_tooltipLastMousePos) > m_tooltipMouseoverRadius
    || m_tooltipParentPane != newTooltipParentPane
    || !m_tooltipParentPane->inWindow(m_tooltipLastMousePos)));

  if (updateTooltip) {
    if (m_activeTooltip) {
      dismiss(observer_ptr<Pane>(m_activeTooltip.get()));
      m_activeTooltip.reset();
      m_tooltipParentPane = {};
    }

    m_tooltipShowTimer.reset();
    if (newTooltipParentPane) {
      if (auto tooltip = newTooltipParentPane->createTooltip(m_tooltipLastMousePos)) {
        m_activeTooltip = std::move(tooltip);
        m_tooltipParentPane = newTooltipParentPane;
        m_tooltipInitialPosition = m_tooltipLastMousePos;
        displayPane(PaneLayer::Tooltip, *m_activeTooltip);
      }
    }
  }

  if (m_activeTooltip) {
    Vec2I offsetDirection = Vec2I::filled(1);
    Vec2I offsetAdjust = Vec2I();

    if (m_tooltipLastMousePos[0] + m_tooltipMouseOffset[0] + m_activeTooltip->size()[0] > static_cast<int>(m_context.windowWidth()) / m_context.interfaceScale()) {
      offsetDirection[0] = -1;
      offsetAdjust[0] = -m_activeTooltip->size()[0];
    }

    if (m_tooltipLastMousePos[1] + m_tooltipMouseOffset[1] - m_activeTooltip->size()[1] < 0)
      offsetDirection[1] = -1;
    else
      offsetAdjust[1] = -m_activeTooltip->size()[1];

    m_activeTooltip->setPosition(m_tooltipLastMousePos + (offsetAdjust + m_tooltipMouseOffset.piecewiseMultiply(offsetDirection)));
  }

  for (auto& [paneLayer, entries] : m_displayedPanes) {
    List<observer_ptr<Pane>> toDismiss;
    for (auto const& entry : entries) {
      if (entry.pane->isDismissed())
        toDismiss.append(entry.pane);
    }
    for (auto const& pane : toDismiss)
      dismiss(pane);
  }

  for (auto const& [paneLayer, entries] : reverseIterate(m_displayedPanes)) {
    for (auto const& entry : reverseIterate(entries)) {
      entry.pane->tick(dt);
      if (entry.pane->active())
        entry.pane->update(dt);
    }
  }
}

Vec2I PaneManager::windowSize() const {
  return Vec2I(m_context.windowInterfaceSize());
}

Vec2I PaneManager::calculatePaneOffset(observer_ptr<Pane> pane) const {
  Vec2I size = pane->size();
  switch (pane->anchor()) {
    case PaneAnchor::None:
      return pane->anchorOffset();
    case PaneAnchor::BottomLeft:
      return pane->anchorOffset();
    case PaneAnchor::BottomRight:
      return pane->anchorOffset() + Vec2I{windowSize()[0] - size[0], 0};
    case PaneAnchor::TopLeft:
      return pane->anchorOffset() + Vec2I{0, windowSize()[1] - size[1]};
    case PaneAnchor::TopRight:
      return pane->anchorOffset() + (windowSize() - size);
    case PaneAnchor::CenterTop:
      return pane->anchorOffset() + Vec2I{(windowSize()[0] - size[0]) / 2, windowSize()[1] - size[1]};
    case PaneAnchor::CenterBottom:
      return pane->anchorOffset() + Vec2I{(windowSize()[0] - size[0]) / 2, 0};
    case PaneAnchor::CenterLeft:
      return pane->anchorOffset() + Vec2I{0, (windowSize()[1] - size[1]) / 2};
    case PaneAnchor::CenterRight:
      return pane->anchorOffset() + Vec2I{windowSize()[0] - size[0], (windowSize()[1] - size[1]) / 2};
    case PaneAnchor::Center:
      return pane->anchorOffset() + ((windowSize() - size) / 2);
    default:
      return pane->anchorOffset();
  }
}

Vec2I PaneManager::calculateNewInterfacePosition(observer_ptr<Pane> pane, float interfaceScaleRatio) const {
  Vec2F position(pane->relativePosition());
  Vec2F size(pane->size());
  Mat3F scale;
  switch (pane->anchor()) {
    case PaneAnchor::None:
      scale = Mat3F::scaling(interfaceScaleRatio, Vec2F(windowSize()) / 2);
      break;
    case PaneAnchor::BottomLeft:
      scale = Mat3F::scaling(interfaceScaleRatio);
      break;
    case PaneAnchor::BottomRight:
      scale = Mat3F::scaling(interfaceScaleRatio, {size[0], 0});
      break;
    case PaneAnchor::TopLeft:
      scale = Mat3F::scaling(interfaceScaleRatio, {0, size[1]});
      break;
    case PaneAnchor::TopRight:
      scale = Mat3F::scaling(interfaceScaleRatio, size);
      break;
    case PaneAnchor::CenterTop:
      scale = Mat3F::scaling(interfaceScaleRatio, {size[0] / 2, size[1]});
      break;
    case PaneAnchor::CenterBottom:
      scale = Mat3F::scaling(interfaceScaleRatio, {size[0] / 2, 0});
      break;
    case PaneAnchor::CenterLeft:
      scale = Mat3F::scaling(interfaceScaleRatio, {0, size[1] / 2});
      break;
    case PaneAnchor::CenterRight:
      scale = Mat3F::scaling(interfaceScaleRatio, {size[0], size[1] / 2});
      break;
    case PaneAnchor::Center:
      scale = Mat3F::scaling(interfaceScaleRatio, size / 2);
      break;
    default:
      scale = Mat3F::scaling(interfaceScaleRatio, Vec2F(windowSize()) / 2);
  }
  return Vec2I::round((scale * Vec3F(position, 0)).vec2());
}

bool PaneManager::dismiss(observer_ptr<Pane> pane) {
  bool dismissed = false;
  for (auto& [paneLayer, entries] : m_displayedPanes) {
    for (auto i = 0; i < entries.size(); ++i) {
      if (entries[i].pane == pane) {
        auto entry = std::move(entries[i]);
        entries.eraseAt(i);
        dismissed = true;
        entry.pane->dismissed();
        if (entry.onDismiss)
          entry.onDismiss(entry.pane);
        break;
      }
    }
  }
  return dismissed;
}

}

