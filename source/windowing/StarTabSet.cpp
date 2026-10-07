#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarEither.hpp"
#include "StarLexicalCast.hpp"

import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
#include "StarVariant.hpp"
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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
import star.tab_set;
import star.button_widget;
import star.stack_widget;
import star.layout;
import star.flow_layout;
import star.widget_parsing;
import star.gui_reader;


import star.image_metadata_database;

namespace Star {

TabSetWidget::TabSetWidget(TabSetConfig const& tabSetConfig) {
  m_tabSetConfig = tabSetConfig;

  m_tabBar = make_shared<FlowLayout>();
  m_tabBar->setSpacing(m_tabSetConfig.tabButtonSpacing);
  Widget::addChild("tabBar", m_tabBar);

  m_stack = make_shared<StackWidget>();
  addChild("tabs", m_stack);

  markAsContainer();
}

void TabSetWidget::setSize(Vec2I const& size) {
  auto imgMetadata = Root::singleton().imageMetadataDatabase();
  auto tabHeight = max({imgMetadata->imageSize(m_tabSetConfig.tabButtonBaseImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonHoverImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonPressedImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonBaseImageSelected).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonHoverImageSelected).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonPressedImageSelected).y()});

  Widget::setSize(Vec2I(size.x(), max<int>(size.y(), tabHeight)));

  m_tabBar->setSize({size.x(), tabHeight});
  m_tabBar->setPosition({0, size.y() - tabHeight});
  m_stack->setSize({size.x(), size.y() - tabHeight});
}

void TabSetWidget::addTab(String const& widgetName, WidgetPtr widget, String const& title) {
  auto newButton = make_shared<ButtonWidget>();
  newButton->setImages(
      m_tabSetConfig.tabButtonBaseImage, m_tabSetConfig.tabButtonHoverImage, m_tabSetConfig.tabButtonPressedImage);
  newButton->setCheckedImages(m_tabSetConfig.tabButtonBaseImageSelected,
      m_tabSetConfig.tabButtonHoverImageSelected,
      m_tabSetConfig.tabButtonPressedImageSelected);
  newButton->setCheckable(true);
  newButton->setText(title);
  newButton->setTextOffset(m_tabSetConfig.tabButtonTextOffset);
  newButton->setPressedOffset(m_tabSetConfig.tabButtonPressedOffset);

  size_t pageForButton = m_tabBar->numChildren();
  newButton->setCallback([this, pageForButton](Widget*) { tabSelect(pageForButton); });

  m_tabBar->addChild(toString(pageForButton), newButton);
  m_stack->addChild(widgetName, widget);

  if (!m_lastSelected)
    tabSelect(0);
}

size_t TabSetWidget::tabCount() const {
  return m_tabBar->numChildren();
}

void TabSetWidget::tabSelect(size_t page) {
  if (m_lastSelected != page) {
    m_lastSelected = page;
    m_stack->showPage(page);
    for (size_t i = 0; i < m_tabBar->numChildren(); ++i) {
      if (i == page)
        m_tabBar->getChildNum<ButtonWidget>(i)->setChecked(true);
      else
        m_tabBar->getChildNum<ButtonWidget>(i)->setChecked(false);
    }
    if (m_callback)
      m_callback(this);
  } else {
    m_tabBar->getChildNum<ButtonWidget>(page)->setChecked(true);
  }
}

size_t TabSetWidget::selectedTab() const {
  return m_lastSelected.value(NPos);
}

void TabSetWidget::setCallback(WidgetCallbackFunc callback) {
  m_callback = std::move(callback);
}

}
