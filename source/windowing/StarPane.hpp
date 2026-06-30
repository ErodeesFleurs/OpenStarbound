#pragma once

#include "StarWidget.hpp"
#include "StarBiMap.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class Pane;
using PanePtr = SharedPtr<Pane>;
class LuaCallbacks;
class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;
class GuiReader;
using GuiReaderPtr = SharedPtr<GuiReader>;

enum class PaneAnchor {
  None,
  BottomLeft,
  BottomRight,
  TopLeft,
  TopRight,
  CenterBottom,
  CenterTop,
  CenterLeft,
  CenterRight,
  Center
};
extern EnumMap<PaneAnchor> const PaneAnchorNames;

class Pane : public Widget {
public:
  explicit Pane(GuiContext& context);

  struct BGResult {
    String header;
    String body;
    String footer;
  };

  virtual void displayed();
  virtual void dismissed();

  void dismiss();
  [[nodiscard]] bool isDismissed() const;
  [[nodiscard]] bool isDisplayed() const;

  [[nodiscard]] Vec2I centerOffset() const;

  // members are drawn strictly in the order they are added,
  // so add them in the correct order.

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;
  virtual void setFocus(Widget const* focus);
  virtual void removeFocus(Widget const* focus);
  virtual void removeFocus();

  void update(float dt) override;
  virtual void tick(float dt);

  [[nodiscard]] bool dragActive() const;
  [[nodiscard]] Vec2I dragMouseOrigin() const;
  void setDragActive(bool dragActive, Vec2I dragMouseOrigin);
  void drag(Vec2I mousePosition);

  [[nodiscard]] bool inWindow(Vec2I const& position) const;
  [[nodiscard]] bool inDragArea(Vec2I const& position) const;
  [[nodiscard]] Vec2I cursorRelativeToPane(Vec2I const& position) const;

  void setBG(BGResult const& res);
  void setBG(String const& header, String const& body = "", String const& footer = "");
  [[nodiscard]] BGResult getBG() const;

  void lockPosition();
  void unlockPosition();

  void setTitle(WidgetPtr icon, String const& title, String const& subTitle);
  void setTitleString(String const& title, String const& subTitle);
  void setTitleIcon(WidgetPtr icon);
  [[nodiscard]] String title() const;
  [[nodiscard]] String subTitle() const;
  [[nodiscard]] WidgetPtr titleIcon() const;

  [[nodiscard]] Pane* window() override;
  [[nodiscard]] Pane const* window() const override;

  [[nodiscard]] PaneAnchor anchor();
  void setAnchor(PaneAnchor anchor);
  [[nodiscard]] Vec2I anchorOffset() const;
  void setAnchorOffset(Vec2I anchorOffset);
  [[nodiscard]] bool hasDisplayed() const;

  // If a tooltip popup should be created at the given mouse position, return a
  // new pane to be used as the tooltip.
  [[nodiscard]] virtual PanePtr createTooltip(Vec2I const& screenPosition);
  [[nodiscard]] virtual Maybe<String> cursorOverride(Vec2I const& screenPosition);
  [[nodiscard]] virtual Maybe<ItemPtr> shiftItemFromInventory(ItemPtr const& input);

  [[nodiscard]] virtual LuaCallbacks makePaneCallbacks();
protected:
  [[nodiscard]] virtual GuiReaderPtr reader();
  void renderImpl() override;

  String m_bgHeader;
  String m_bgBody;
  String m_bgFooter;

  Vec2I m_footerSize;
  Vec2I m_bodySize;
  Vec2I m_headerSize;

  bool m_dismissed = true;
  bool m_dragActive = false;
  Vec2I m_dragMouseOrigin;
  bool m_lockPosition = false;
  Vec2I m_centerOffset;

  WidgetPtr m_mouseOver;
  WidgetPtr m_clickDown;
  WidgetPtr m_focusWidget;

  WidgetPtr m_icon;
  String m_title;
  String m_subTitle;
  TextStyle m_textStyle;
  Vec2I m_iconOffset;
  Vec2I m_titleOffset;
  Vec2I m_subTitleOffset;
  Color m_titleColor;
  Color m_subTitleColor;

  PaneAnchor m_anchor = PaneAnchor::None;
  Vec2I m_anchorOffset;
  bool m_hasDisplayed = false;

  struct PlayingSound {
    String audioName;
    AudioInstancePtr instance;
  };
  List<PlayingSound> m_playingSounds;
};

}
