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
import star.button_group;

namespace Star {

class FlowLayout;
using FlowLayoutPtr = std::shared_ptr<FlowLayout>;
using FlowLayoutConstPtr = std::shared_ptr<FlowLayout const>;
using FlowLayoutWeakPtr = std::weak_ptr<FlowLayout>;
using FlowLayoutConstWeakPtr = std::weak_ptr<FlowLayout const>;
using FlowLayoutUPtr = std::unique_ptr<FlowLayout>;
using FlowLayoutConstUPtr = std::unique_ptr<FlowLayout const>;
class StackWidget;
using StackWidgetPtr = std::shared_ptr<StackWidget>;
using StackWidgetConstPtr = std::shared_ptr<StackWidget const>;
using StackWidgetWeakPtr = std::weak_ptr<StackWidget>;
using StackWidgetConstWeakPtr = std::weak_ptr<StackWidget const>;
using StackWidgetUPtr = std::unique_ptr<StackWidget>;
using StackWidgetConstUPtr = std::unique_ptr<StackWidget const>;

class TabBarWidget;
using TabBarWidgetPtr = std::shared_ptr<TabBarWidget>;
using TabBarWidgetConstPtr = std::shared_ptr<TabBarWidget const>;
using TabBarWidgetWeakPtr = std::weak_ptr<TabBarWidget>;
using TabBarWidgetConstWeakPtr = std::weak_ptr<TabBarWidget const>;
using TabBarWidgetUPtr = std::unique_ptr<TabBarWidget>;
using TabBarWidgetConstUPtr = std::unique_ptr<TabBarWidget const>;
class TabSetWidget;
using TabSetWidgetPtr = std::shared_ptr<TabSetWidget>;
using TabSetWidgetConstPtr = std::shared_ptr<TabSetWidget const>;
using TabSetWidgetWeakPtr = std::weak_ptr<TabSetWidget>;
using TabSetWidgetConstWeakPtr = std::weak_ptr<TabSetWidget const>;
using TabSetWidgetUPtr = std::unique_ptr<TabSetWidget>;
using TabSetWidgetConstUPtr = std::unique_ptr<TabSetWidget const>;

struct TabSetConfig {
  String tabButtonBaseImage;
  String tabButtonHoverImage;
  String tabButtonPressedImage;
  String tabButtonBaseImageSelected;
  String tabButtonHoverImageSelected;
  String tabButtonPressedImageSelected;
  Vec2I tabButtonPressedOffset;
  Vec2I tabButtonTextOffset;
  Vec2I tabButtonSpacing;
};

class TabSetWidget : public Widget {
public:
  TabSetWidget(TabSetConfig const& tabSetconfig);

  virtual void setSize(Vec2I const& size) override;

  void addTab(String const& widgetName, WidgetPtr widget, String const& title);

  std::size_t tabCount() const;
  void tabSelect(std::size_t page);
  std::size_t selectedTab() const;

  // Callback is called when the tab changes
  void setCallback(WidgetCallbackFunc callback);

private:
  TabSetConfig m_tabSetConfig;
  FlowLayoutPtr m_tabBar;
  StackWidgetPtr m_stack;
  WidgetCallbackFunc m_callback;
  Maybe<std::size_t> m_lastSelected;
};

}

export module star.tab_set;

export namespace Star {
  using ::Star::TabSetConfig;
  using ::Star::TabSetWidget;
  using ::Star::FlowLayout;
  using ::Star::FlowLayoutPtr;
  using ::Star::FlowLayoutConstPtr;
  using ::Star::FlowLayoutWeakPtr;
  using ::Star::FlowLayoutConstWeakPtr;
  using ::Star::FlowLayoutUPtr;
  using ::Star::FlowLayoutConstUPtr;
  using ::Star::StackWidget;
  using ::Star::StackWidgetPtr;
  using ::Star::StackWidgetConstPtr;
  using ::Star::StackWidgetWeakPtr;
  using ::Star::StackWidgetConstWeakPtr;
  using ::Star::StackWidgetUPtr;
  using ::Star::StackWidgetConstUPtr;
  using ::Star::TabBarWidget;
  using ::Star::TabBarWidgetPtr;
  using ::Star::TabBarWidgetConstPtr;
  using ::Star::TabBarWidgetWeakPtr;
  using ::Star::TabBarWidgetConstWeakPtr;
  using ::Star::TabBarWidgetUPtr;
  using ::Star::TabBarWidgetConstUPtr;
  using ::Star::TabSetWidgetPtr;
  using ::Star::TabSetWidgetConstPtr;
  using ::Star::TabSetWidgetWeakPtr;
  using ::Star::TabSetWidgetConstWeakPtr;
  using ::Star::TabSetWidgetUPtr;
  using ::Star::TabSetWidgetConstUPtr;
}
