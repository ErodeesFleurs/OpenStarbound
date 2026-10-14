import std;
import star.memory;
import star.json;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;
import star.string;
import star.text;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.either;
import star.lexical_cast;

import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
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

  m_tabBar = std::make_shared<FlowLayout>();
  m_tabBar->setSpacing(m_tabSetConfig.tabButtonSpacing);
  Widget::addChild("tabBar", m_tabBar);

  m_stack = std::make_shared<StackWidget>();
  addChild("tabs", m_stack);

  markAsContainer();
}

void TabSetWidget::setSize(Vec2I const& size) {
  auto imgMetadata = Root::singleton().imageMetadataDatabase();
  auto tabHeight = std::max({imgMetadata->imageSize(m_tabSetConfig.tabButtonBaseImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonHoverImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonPressedImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonBaseImageSelected).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonHoverImageSelected).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonPressedImageSelected).y()});

  Widget::setSize(Vec2I(size.x(), std::max<int>(size.y(), tabHeight)));

  m_tabBar->setSize({size.x(), tabHeight});
  m_tabBar->setPosition({0, size.y() - tabHeight});
  m_stack->setSize({size.x(), size.y() - tabHeight});
}

void TabSetWidget::addTab(String const& widgetName, WidgetPtr widget, String const& title) {
  auto newButton = std::make_shared<ButtonWidget>();
  newButton->setImages(
      m_tabSetConfig.tabButtonBaseImage, m_tabSetConfig.tabButtonHoverImage, m_tabSetConfig.tabButtonPressedImage);
  newButton->setCheckedImages(m_tabSetConfig.tabButtonBaseImageSelected,
      m_tabSetConfig.tabButtonHoverImageSelected,
      m_tabSetConfig.tabButtonPressedImageSelected);
  newButton->setCheckable(true);
  newButton->setText(title);
  newButton->setTextOffset(m_tabSetConfig.tabButtonTextOffset);
  newButton->setPressedOffset(m_tabSetConfig.tabButtonPressedOffset);

  std::size_t pageForButton = m_tabBar->numChildren();
  newButton->setCallback([this, pageForButton](Widget*) { tabSelect(pageForButton); });

  m_tabBar->addChild(toString(pageForButton), newButton);
  m_stack->addChild(widgetName, widget);

  if (!m_lastSelected)
    tabSelect(0);
}

std::size_t TabSetWidget::tabCount() const {
  return m_tabBar->numChildren();
}

void TabSetWidget::tabSelect(std::size_t page) {
  if (m_lastSelected != page) {
    m_lastSelected = page;
    m_stack->showPage(page);
    for (std::size_t i = 0; i < m_tabBar->numChildren(); ++i) {
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

std::size_t TabSetWidget::selectedTab() const {
  return m_lastSelected.value(NPos);
}

void TabSetWidget::setCallback(WidgetCallbackFunc callback) {
  m_callback = std::move(callback);
}

}
