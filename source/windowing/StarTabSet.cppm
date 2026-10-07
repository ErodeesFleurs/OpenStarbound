module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
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
import star.button_group;

namespace Star {

STAR_CLASS(FlowLayout);
STAR_CLASS(StackWidget);

STAR_CLASS(TabBarWidget);
STAR_CLASS(TabSetWidget);

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

  size_t tabCount() const;
  void tabSelect(size_t page);
  size_t selectedTab() const;

  // Callback is called when the tab changes
  void setCallback(WidgetCallbackFunc callback);

private:
  TabSetConfig m_tabSetConfig;
  FlowLayoutPtr m_tabBar;
  StackWidgetPtr m_stack;
  WidgetCallbackFunc m_callback;
  Maybe<size_t> m_lastSelected;
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
