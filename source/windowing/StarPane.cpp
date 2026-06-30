#include "StarPane.hpp"
#include "StarJsonExtra.hpp"
#include "StarAssets.hpp"
#include "StarWidgetLuaBindings.hpp"
#include "StarLuaConverters.hpp"
#include "StarImageWidget.hpp"
#include "StarItemDatabase.hpp"
#include "StarGuiReader.hpp"

namespace Star {

EnumMap<PaneAnchor> const PaneAnchorNames{
    {PaneAnchor::None, "none"},
    {PaneAnchor::BottomLeft, "bottomLeft"},
    {PaneAnchor::BottomRight, "bottomRight"},
    {PaneAnchor::TopLeft, "topLeft"},
    {PaneAnchor::TopRight, "topRight"},
    {PaneAnchor::CenterBottom, "centerBottom"},
    {PaneAnchor::CenterTop, "centerTop"},
    {PaneAnchor::CenterLeft, "centerLeft"},
    {PaneAnchor::CenterRight, "centerRight"},
    {PaneAnchor::Center, "center"},
};

Pane::Pane(GuiContext& context) : Widget(context) {
  m_centerOffset = Vec2I();
  m_anchorOffset = Vec2I();
  m_visible = false;

  auto& guiContext = this->context();
  auto const& assets = guiContext.assets();
  m_textStyle = assets->json("/interface.config:paneTextStyle");
  m_iconOffset = jsonToVec2I(assets->json("/interface.config:paneIconOffset"));
  m_titleOffset = jsonToVec2I(assets->json("/interface.config:paneTitleOffset"));
  m_subTitleOffset = jsonToVec2I(assets->json("/interface.config:paneSubTitleOffset"));
  m_titleColor = jsonToColor(assets->json("/interface.config:paneTitleColor"));
  m_subTitleColor = jsonToColor(assets->json("/interface.config:paneSubTitleColor"));
}

void Pane::displayed() {
  m_dismissed = false;
  m_hasDisplayed = true;
  show();
}

void Pane::dismissed() {
  if (m_clickDown)
    m_clickDown->mouseOut();
  m_clickDown = nullptr;
  if (m_mouseOver)
    m_mouseOver->mouseOut();
  m_mouseOver = nullptr;
  hide();
  m_dismissed = true;
}

void Pane::dismiss() {
  m_dismissed = true;
}

bool Pane::isDismissed() const {
  return m_dismissed;
}

bool Pane::isDisplayed() const {
  return !m_dismissed;
}

bool Pane::sendEvent(InputEvent const& event) {
  if (m_visible) {
    if (event.is<MouseButtonDownEvent>() || event.is<MouseButtonUpEvent>() || event.is<MouseMoveEvent>()
        || event.is<MouseWheelEvent>()) {
      Vec2I mousePos = *context().mousePosition(event);
      // First, handle preliminary mouse out / click up events
      if (m_mouseOver) {
        if (!m_mouseOver->inMember(mousePos) || !m_mouseOver->active()) {
          m_mouseOver->mouseOut();
          m_mouseOver = nullptr;
        }
      }

      if (event.is<MouseButtonUpEvent>())
        m_clickDown = nullptr;

      Widget* newClickDown = nullptr;
      Widget* newMouseOver = nullptr;
      Widget* newFocusWidget = nullptr;

      List<WidgetRef<Widget>> validWidgets;
      // gather valid widgets into our own list because any of them could mutate
      // m_members while processing the event and fuck everything up
      for (auto const& widget : reverseIterate(m_members)) {
        if (widget->inMember(mousePos) && widget->active() && widget->interactive())
          validWidgets.append(WidgetRef<Widget>(*widget));
      }

      // Then, go through widgets in highest to lowest z-order and handle mouse
      // over, focus, and capture events.
      for (auto const& widget : validWidgets) {
        auto child = getChildAt(mousePos);
        if (child && child->active() && child->interactive()) {
          if (event.is<MouseButtonDownEvent>()
              && (event.get<MouseButtonDownEvent>().mouseButton == MouseButton::Left
                      || event.get<MouseButtonDownEvent>().mouseButton == MouseButton::Right)) {
            if (!newClickDown)
              newClickDown = child.get();

            if (!newFocusWidget)
              newFocusWidget = child.get();
          }

          if (!newMouseOver)
            newMouseOver = child.get();
        }
      }

      if (m_clickDown.get() != newClickDown)
        m_clickDown = newClickDown ? WidgetRef<Widget>(*newClickDown) : WidgetRef<Widget>();

      if (m_mouseOver.get() != newMouseOver) {
        if (m_mouseOver)
          m_mouseOver->mouseOut();
        m_mouseOver = newMouseOver ? WidgetRef<Widget>(*newMouseOver) : WidgetRef<Widget>();
        if (m_mouseOver) {
          if (m_clickDown.get() == m_mouseOver.get())
            m_mouseOver->mouseReturnStillDown();
          else
            m_mouseOver->mouseOver();
        }
      }

      if (newFocusWidget && m_focusWidget.get() != newFocusWidget) {
        if (m_focusWidget)
          m_focusWidget->blur();
        m_focusWidget = WidgetRef<Widget>(*newFocusWidget);
        m_focusWidget->focus();
      }

      // Finally go through widgets in highest to lowest z-order and send the
      // raw event, stopping further processing if the widget consumes it.
      for (auto const& widget : validWidgets) {
        if (widget->sendEvent(event))
          return true;
      }
    }

    if (event.is<MouseButtonDownEvent>()) {
      Vec2I mousePos = *context().mousePosition(event);
      if (inDragArea(mousePos) && !m_lockPosition) {
        setDragActive(true, mousePos);
        return true;
      }
      if (inWindow(mousePos))
        return true;
    }

    if (m_focusWidget) {
      if (m_focusWidget->sendEvent(event))
        return true;
    }
  }
  return false;
}

void Pane::setFocus(Widget& focus) {
  if (m_focusWidget.get() == &focus)
    return;
  if (m_focusWidget)
    m_focusWidget->blur();
  m_focusWidget = WidgetRef<Widget>(focus);
  m_focusWidget->focus();
}

void Pane::removeFocus(Widget const* focus) {
  if (m_focusWidget.get() == focus)
    m_focusWidget = nullptr;
}

void Pane::removeFocus() {
  m_focusWidget = nullptr;
}

Pane const* Pane::window() const {
  return this;
}

Pane* Pane::window() {
  return this;
}

void Pane::update(float dt) {
  if (m_visible) {
    for (auto const& widget : m_members) {
      widget->update(dt);
      if (m_focusWidget.get() != widget.get() || !widget->hasFocus()) {
        m_focusWidget = nullptr;
        widget->blur();
      }
    }
  }
}

void Pane::tick(float) {
  m_playingSounds.filter([](auto const& playingSound) {
    return playingSound.instance->finished() == false;
  });
}

bool Pane::dragActive() const {
  return m_dragActive;
}

Vec2I Pane::dragMouseOrigin() const {
  return m_dragMouseOrigin;
}

void Pane::setDragActive(bool dragActive, Vec2I dragMouseOrigin) {
  m_dragActive = dragActive;
  m_dragMouseOrigin = dragMouseOrigin;
}

void Pane::drag(Vec2I mousePosition) {
  Vec2I delta = mousePosition - m_dragMouseOrigin;
  setPosition(relativePosition() + delta);
  m_dragMouseOrigin += delta;
}

bool Pane::inWindow(Vec2I const& position) const {
  return screenBoundRect().contains(position);
}

bool Pane::inDragArea(Vec2I const& position) const {
  return inWindow(position) && (position[1] < (this->position()[1] + m_footerSize[1])
                                   || position[1] > (this->position()[1] + (m_footerSize[1] + m_bodySize[1])));
}

Vec2I Pane::cursorRelativeToPane(Vec2I const& position) const {
  return position - this->position();
}

Vec2I Pane::centerOffset() const {
  return m_centerOffset;
}

void Pane::setBG(BGResult const& res) {
  setBG(res.header, res.body, res.footer);
}

void Pane::setBG(String const& header, String const& body, String const& footer) {
  m_bgHeader = header;
  m_bgBody = body;
  m_bgFooter = footer;
  if (m_bgHeader != "") {
    m_headerSize = Vec2I(context().textureSize(m_bgHeader));
  } else {
    m_headerSize = {};
  }
  if (m_bgBody != "") {
    m_bodySize = Vec2I(context().textureSize(m_bgBody));
  } else {
    m_bodySize = {};
  }
  if (m_bgFooter != "") {
    m_footerSize = Vec2I(context().textureSize(m_bgFooter));
  } else {
    m_footerSize = {};
  }

  setSize(Vec2I(std::max(std::max(m_headerSize[0], m_bodySize[0]), m_footerSize[0]),
      m_headerSize[1] + m_bodySize[1] + m_footerSize[1]));
}

Pane::BGResult Pane::getBG() const {
  return {m_bgHeader, m_bgBody, m_bgFooter};
}

void Pane::lockPosition() {
  m_lockPosition = true;
}

void Pane::unlockPosition() {
  m_lockPosition = false;
}

void Pane::setTitle(UniquePtr<Widget> icon, String const& title, String const& subTitle) {
  m_icon = std::move(icon);
  m_title = title;
  m_subTitle = subTitle;
  if (m_icon) {
    m_icon->setParent(observer_ptr<Widget>(this));
    m_icon->show();
  }
}

void Pane::setTitleString(String const& title, String const& subTitle) {
  m_title = title;
  m_subTitle = subTitle;
}

void Pane::setTitleIcon(UniquePtr<Widget> icon) {
  m_icon = std::move(icon);
  if (m_icon) {
    m_icon->setParent(observer_ptr<Widget>(this));
    m_icon->show();
  }
}

String Pane::title() const {
  return m_title;
}

String Pane::subTitle() const {
  return m_subTitle;
}

observer_ptr<Widget> Pane::titleIcon() const {
  return observer_ptr<Widget>(m_icon.get());
}

PaneAnchor Pane::anchor() {
  return m_anchor;
}

void Pane::setAnchor(PaneAnchor anchor) {
  m_anchor = anchor;
}

Vec2I Pane::anchorOffset() const {
  return m_anchorOffset;
}

void Pane::setAnchorOffset(Vec2I anchorOffset) {
  m_anchorOffset = anchorOffset;
}

bool Pane::hasDisplayed() const {
  return m_hasDisplayed;
}

UniquePtr<Pane> Pane::createTooltip(Vec2I const&) {
  return {};
}

Maybe<String> Pane::cursorOverride(Vec2I const&) {
  return {};
}

Maybe<ItemPtr> Pane::shiftItemFromInventory(ItemPtr const&) {
  return {};
}

LuaCallbacks Pane::makePaneCallbacks() {
  LuaCallbacks callbacks;

  callbacks.registerCallback("toWidget", [this]() -> LuaCallbacks {
    return LuaBindings::makeWidgetCallbacks(*this, reader());
  });

  callbacks.registerCallback("dismiss", [this]() { dismiss(); });

  callbacks.registerCallback("playSound",
    [this](String const& audio, Maybe<int> loops, Maybe<float> volume) {
      auto const& assets = context().assets();
      auto audioInstance = make_shared<AudioInstance>(*assets->audio(audio));
      audioInstance->setVolume(volume.value(1.0));
      audioInstance->setLoops(loops.value(0));
      context().playAudio(audioInstance);
      m_playingSounds.append(PlayingSound{audio, std::move(audioInstance)});
    });

  callbacks.registerCallback("stopAllSounds", [this](Maybe<String> const& audio) {
      m_playingSounds.filter([audio](auto const& playingSound) {
        if (!audio || playingSound.audioName == *audio) {
          playingSound.instance->stop();
          return false;
        }
        return true;
      });
    });

  callbacks.registerCallback("setTitle", [this](String const& title, String const& subTitle) {
      setTitleString(title, subTitle);
    });

  callbacks.registerCallback("setTitleIcon", [this](String const& image) {
      auto iconPtr = titleIcon();
      if (iconPtr) {
        if (auto icon = as<ImageWidget>(WidgetRef<Widget>(*iconPtr)))
          icon->setImage(image);
      }
    });

  callbacks.registerCallback("getPosition", [this]() -> Vec2I          { return relativePosition(); });
  callbacks.registerCallback("setPosition", [this](Vec2I const& position) { setPosition(position);  });
  callbacks.registerCallback("getSize", [this]() -> Vec2I         {  return size();  });
  callbacks.registerCallback("setSize", [this](Vec2I const& size) { setSize(size);   });

  callbacks.registerCallback("addWidget", [this](Json const& newWidgetConfig, Maybe<String> const& newWidgetName) -> Maybe<LuaCallbacks> {
      String name = newWidgetName.value(toString(Random::randu64()));
      auto newWidget = reader()->makeSingle(name, newWidgetConfig);
      if (newWidget) {
        auto* rawPtr = newWidget.get();
        this->addChild(name, std::move(newWidget));
        return LuaBindings::makeWidgetCallbacks(*rawPtr, reader());
      } else {
        return {};
      }
    });

  callbacks.registerCallback("removeWidget", [this](String const& widgetName) -> bool
    { return this->removeChild(widgetName); });

  auto* guiContext = &context();
  callbacks.registerCallback("scale", [guiContext]() { return guiContext->interfaceScale(); });
  callbacks.registerCallback("isDisplayed", [this]() { return isDisplayed(); });
  callbacks.registerCallback("hasFocus", [this]() { return hasFocus(); });
  callbacks.registerCallback("show", [this]() { show(); });
  callbacks.registerCallback("hide", [this]() { hide(); });
  callbacks.registerCallback("anchor", [this]() { return PaneAnchorNames.getRight(anchor()); });  
  callbacks.registerCallback("setAnchor", [this](String anchorName) {
    setAnchor(PaneAnchorNames.getLeft(anchorName)); 
  });
  callbacks.registerCallback("anchorOffset", [this]() { return anchorOffset(); });
  callbacks.registerCallback("setAnchorOffset", [this](Vec2I offset) { setAnchorOffset(offset); });
  callbacks.registerCallback("getScreenPosition", [this]() -> Vec2I {
    Vec2I windowSize = Vec2I(context().windowInterfaceSize());
    Vec2I sz = size();
    Vec2I offset;
    switch (anchor()) {
    case PaneAnchor::None:
    case PaneAnchor::BottomLeft:
      offset = anchorOffset();
      break;
    case PaneAnchor::BottomRight:
      offset = anchorOffset() + Vec2I{windowSize[0] - sz[0], 0};
      break;
    case PaneAnchor::TopLeft:
      offset = anchorOffset() + Vec2I{0, windowSize[1] - sz[1]};
      break;
    case PaneAnchor::TopRight:
      offset = anchorOffset() + (windowSize - sz);
      break;
    case PaneAnchor::CenterTop:
      offset = anchorOffset() + Vec2I{(windowSize[0] - sz[0]) / 2, windowSize[1] - sz[1]};
      break;
    case PaneAnchor::CenterBottom:
      offset = anchorOffset() + Vec2I{(windowSize[0] - sz[0]) / 2, 0};
      break;
    case PaneAnchor::CenterLeft:
      offset = anchorOffset() + Vec2I{0, (windowSize[1] - sz[1]) / 2};
      break;
    case PaneAnchor::CenterRight:
      offset = anchorOffset() + Vec2I{windowSize[0] - sz[0], (windowSize[1] - sz[1]) / 2};
      break;
    case PaneAnchor::Center:
      offset = anchorOffset() + ((windowSize - sz) / 2);
      break;
    default:
      offset = anchorOffset();
    }
    return offset + relativePosition();
  });

  callbacks.registerCallback("setScreenPosition", [this](Vec2I screenPos) {
    Vec2I windowSize = Vec2I(context().windowInterfaceSize());
    Vec2I sz = size();
    Vec2I offset;
    switch (anchor()) {
    case PaneAnchor::None:
    case PaneAnchor::BottomLeft:
      offset = anchorOffset();
      break;
    case PaneAnchor::BottomRight:
      offset = anchorOffset() + Vec2I{windowSize[0] - sz[0], 0};
      break;
    case PaneAnchor::TopLeft:
      offset = anchorOffset() + Vec2I{0, windowSize[1] - sz[1]};
      break;
    case PaneAnchor::TopRight:
      offset = anchorOffset() + (windowSize - sz);
      break;
    case PaneAnchor::CenterTop:
      offset = anchorOffset() + Vec2I{(windowSize[0] - sz[0]) / 2, windowSize[1] - sz[1]};
      break;
    case PaneAnchor::CenterBottom:
      offset = anchorOffset() + Vec2I{(windowSize[0] - sz[0]) / 2, 0};
      break;
    case PaneAnchor::CenterLeft:
      offset = anchorOffset() + Vec2I{0, (windowSize[1] - sz[1]) / 2};
      break;
    case PaneAnchor::CenterRight:
      offset = anchorOffset() + Vec2I{windowSize[0] - sz[0], (windowSize[1] - sz[1]) / 2};
      break;
    case PaneAnchor::Center:
      offset = anchorOffset() + ((windowSize - sz) / 2);
      break;
    default:
      offset = anchorOffset();
    }

    setPosition(screenPos - offset);
  });

  return callbacks;
}

GuiReaderPtr Pane::reader() {
  return make_shared<GuiReader>(context());
}

void Pane::renderImpl() {
  if (m_bgFooter != "")
    context().drawInterfaceQuad(m_bgFooter, Vec2F(position()));

  if (m_bgBody != "")
    context().drawInterfaceQuad(m_bgBody, Vec2F(position()) + Vec2F(0, m_footerSize[1]));

  if (m_bgHeader != "") {
    auto headerPos = Vec2F(position()) + Vec2F(0, m_footerSize[1] + m_bodySize[1]);
    context().drawInterfaceQuad(m_bgHeader, headerPos);

    if (m_icon) {
      m_icon->setPosition(Vec2I(0, m_footerSize[1] + m_bodySize[1]) + m_iconOffset);
      m_icon->render(m_drawingArea);
      context().resetInterfaceScissorRect();
    }

    context().setTextStyle(m_textStyle);
    context().setFontColor(m_titleColor.toRgba());
    context().setFontMode(FontMode::Shadow);
    context().renderInterfaceText(m_title, {headerPos + Vec2F(m_titleOffset)});
    context().setFontColor(m_subTitleColor.toRgba());
    context().renderInterfaceText(m_subTitle, {headerPos + Vec2F(m_subTitleOffset)});
    context().clearTextStyle();
  }
}

}
