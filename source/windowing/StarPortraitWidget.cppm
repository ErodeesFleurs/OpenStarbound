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
import star.rect;
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

class PortraitEntity;
using PortraitEntityPtr = std::shared_ptr<PortraitEntity>;
using PortraitEntityConstPtr = std::shared_ptr<PortraitEntity const>;
using PortraitEntityWeakPtr = std::weak_ptr<PortraitEntity>;
using PortraitEntityConstWeakPtr = std::weak_ptr<PortraitEntity const>;
using PortraitEntityUPtr = std::unique_ptr<PortraitEntity>;
using PortraitEntityConstUPtr = std::unique_ptr<PortraitEntity const>;

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
class PortraitWidget;
using PortraitWidgetPtr = std::shared_ptr<PortraitWidget>;
using PortraitWidgetConstPtr = std::shared_ptr<PortraitWidget const>;
using PortraitWidgetWeakPtr = std::weak_ptr<PortraitWidget>;
using PortraitWidgetConstWeakPtr = std::weak_ptr<PortraitWidget const>;
using PortraitWidgetUPtr = std::unique_ptr<PortraitWidget>;
using PortraitWidgetConstUPtr = std::unique_ptr<PortraitWidget const>;

class PortraitWidget : public Widget {
public:
  PortraitWidget(PortraitEntityPtr entity, PortraitMode mode = PortraitMode::Full);
  PortraitWidget();
  virtual ~PortraitWidget() {}

  void setEntity(PortraitEntityPtr entity);
  void setMode(PortraitMode mode);
  void setScale(float scale);
  void setIconMode();
  void setRenderHumanoid(bool);
  bool sendEvent(InputEvent const& event);

protected:
  virtual RectI getScissorRect() const;
  virtual void renderImpl();

private:
  void init();
  void updateSize();

  PortraitEntityPtr m_entity;
  PortraitMode m_portraitMode;
  AssetPath m_noEntityImageFull;
  AssetPath m_noEntityImagePart;
  float m_scale;

  bool m_renderHumanoid;
  bool m_iconMode;
  AssetPath m_iconImage;
  Vec2I m_iconOffset;
};

}

export module star.portrait_widget;

export namespace Star {
  using ::Star::PortraitWidget;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::PortraitWidgetPtr;
  using ::Star::PortraitWidgetConstPtr;
  using ::Star::PortraitWidgetWeakPtr;
  using ::Star::PortraitWidgetConstWeakPtr;
  using ::Star::PortraitWidgetUPtr;
  using ::Star::PortraitWidgetConstUPtr;
  using ::Star::PortraitEntity;
  using ::Star::PortraitEntityPtr;
  using ::Star::PortraitEntityConstPtr;
  using ::Star::PortraitEntityWeakPtr;
  using ::Star::PortraitEntityConstWeakPtr;
  using ::Star::PortraitEntityUPtr;
  using ::Star::PortraitEntityConstUPtr;
}
