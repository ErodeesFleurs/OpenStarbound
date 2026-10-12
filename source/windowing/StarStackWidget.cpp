#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
import star.ref_ptr;
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarList.hpp"
#include "StarMap.hpp"
import star.either;

import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
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
import star.stack_widget;

namespace Star {

void StackWidget::showPage(size_t page) {
  if (m_shownPage)
    m_shownPage->hide();
  m_shownPage = m_members[page];
  m_page = makeLeft(page);
  if (m_shownPage)
    m_shownPage->show();
}

void StackWidget::showPage(String const& name) {
  if (m_shownPage)
    m_shownPage->hide();
  m_shownPage = m_memberHash.get(name);
  m_page = makeRight(name);
  if (m_shownPage)
    m_shownPage->show();
}

Either<size_t, String> StackWidget::currentPage() const {
  return m_page;
}

void StackWidget::addChild(String const& name, WidgetPtr member) {
  Widget::addChild(name, member);
  if (m_members.size() != 1)
    member->hide();
  else
    showPage(0);
}

}
