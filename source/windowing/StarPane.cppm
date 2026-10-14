module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.color;
import star.font;
import star.variant;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
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

namespace Star {

class Pane;
using PanePtr = std::shared_ptr<Pane>;
using PaneConstPtr = std::shared_ptr<Pane const>;
using PaneWeakPtr = std::weak_ptr<Pane>;
using PaneConstWeakPtr = std::weak_ptr<Pane const>;
using PaneUPtr = std::unique_ptr<Pane>;
using PaneConstUPtr = std::unique_ptr<Pane const>;
class LuaCallbacks;
using LuaCallbacksPtr = std::shared_ptr<LuaCallbacks>;
using LuaCallbacksConstPtr = std::shared_ptr<LuaCallbacks const>;
using LuaCallbacksWeakPtr = std::weak_ptr<LuaCallbacks>;
using LuaCallbacksConstWeakPtr = std::weak_ptr<LuaCallbacks const>;
using LuaCallbacksUPtr = std::unique_ptr<LuaCallbacks>;
using LuaCallbacksConstUPtr = std::unique_ptr<LuaCallbacks const>;
class AudioInstance;
using AudioInstancePtr = std::shared_ptr<AudioInstance>;
using AudioInstanceConstPtr = std::shared_ptr<AudioInstance const>;
using AudioInstanceWeakPtr = std::weak_ptr<AudioInstance>;
using AudioInstanceConstWeakPtr = std::weak_ptr<AudioInstance const>;
using AudioInstanceUPtr = std::unique_ptr<AudioInstance>;
using AudioInstanceConstUPtr = std::unique_ptr<AudioInstance const>;
class GuiReader;
using GuiReaderPtr = std::shared_ptr<GuiReader>;
using GuiReaderConstPtr = std::shared_ptr<GuiReader const>;
using GuiReaderWeakPtr = std::weak_ptr<GuiReader>;
using GuiReaderConstWeakPtr = std::weak_ptr<GuiReader const>;
using GuiReaderUPtr = std::unique_ptr<GuiReader>;
using GuiReaderConstUPtr = std::unique_ptr<GuiReader const>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<Item const>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<Item const>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<Item const>;

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
  Pane();

  struct BGResult {
    String header;
    String body;
    String footer;
  };

  virtual void displayed();
  virtual void dismissed();

  void dismiss();
  bool isDismissed() const;
  bool isDisplayed() const;

  Vec2I centerOffset() const;

  // members are drawn strictly in the order they are added,
  // so add them in the correct order.

  virtual bool sendEvent(InputEvent const& event);
  virtual void setFocus(Widget const* focus);
  virtual void removeFocus(Widget const* focus);
  virtual void removeFocus();

  virtual void update(float dt);
  virtual void tick(float dt);

  bool dragActive() const;
  Vec2I dragMouseOrigin() const;
  void setDragActive(bool dragActive, Vec2I dragMouseOrigin);
  void drag(Vec2I mousePosition);

  bool inWindow(Vec2I const& position) const;
  bool inDragArea(Vec2I const& position) const;
  Vec2I cursorRelativeToPane(Vec2I const& position) const;

  void setBG(BGResult const& res);
  void setBG(String const& header, String const& body = "", String const& footer = "");
  BGResult getBG() const;

  void lockPosition();
  void unlockPosition();

  void setTitle(WidgetPtr icon, String const& title, String const& subTitle);
  void setTitleString(String const& title, String const& subTitle);
  void setTitleIcon(WidgetPtr icon);
  String title() const;
  String subTitle() const;
  WidgetPtr titleIcon() const;

  virtual Pane* window();
  virtual Pane const* window() const;

  PaneAnchor anchor();
  void setAnchor(PaneAnchor anchor);
  Vec2I anchorOffset() const;
  void setAnchorOffset(Vec2I anchorOffset);
  bool hasDisplayed() const;

  // If a tooltip popup should be created at the given mouse position, return a
  // new pane to be used as the tooltip.
  virtual PanePtr createTooltip(Vec2I const& screenPosition);
  virtual Maybe<String> cursorOverride(Vec2I const& screenPosition);
  virtual Maybe<ItemPtr> shiftItemFromInventory(ItemPtr const& input);

  virtual LuaCallbacks makePaneCallbacks();
protected:
  virtual GuiReaderPtr reader();
  virtual void renderImpl();

  String m_bgHeader;
  String m_bgBody;
  String m_bgFooter;

  Vec2I m_footerSize;
  Vec2I m_bodySize;
  Vec2I m_headerSize;

  bool m_dismissed;
  bool m_dragActive;
  Vec2I m_dragMouseOrigin;
  bool m_lockPosition;
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

  PaneAnchor m_anchor;
  Vec2I m_anchorOffset;
  bool m_hasDisplayed;

  List<std::pair<String, AudioInstancePtr>> m_playingSounds;
};

// GCC 16 needs the existing key function in this complete-definition TU for RTTI.
void Pane::displayed() {
  m_dismissed = false;
  m_hasDisplayed = true;
  show();
}

}

export module star.pane;

export namespace Star {
  using ::Star::PaneAnchor;
  using ::Star::Pane;
  using ::Star::PanePtr;
  using ::Star::PaneConstPtr;
  using ::Star::PaneWeakPtr;
  using ::Star::PaneConstWeakPtr;
  using ::Star::PaneUPtr;
  using ::Star::PaneConstUPtr;
  using ::Star::LuaCallbacks;
  using ::Star::LuaCallbacksPtr;
  using ::Star::LuaCallbacksConstPtr;
  using ::Star::LuaCallbacksWeakPtr;
  using ::Star::LuaCallbacksConstWeakPtr;
  using ::Star::LuaCallbacksUPtr;
  using ::Star::LuaCallbacksConstUPtr;
  using ::Star::AudioInstance;
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::AudioInstanceWeakPtr;
  using ::Star::AudioInstanceConstWeakPtr;
  using ::Star::AudioInstanceUPtr;
  using ::Star::AudioInstanceConstUPtr;
  using ::Star::GuiReader;
  using ::Star::GuiReaderPtr;
  using ::Star::GuiReaderConstPtr;
  using ::Star::GuiReaderWeakPtr;
  using ::Star::GuiReaderConstWeakPtr;
  using ::Star::GuiReaderUPtr;
  using ::Star::GuiReaderConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::PaneAnchorNames;
}
