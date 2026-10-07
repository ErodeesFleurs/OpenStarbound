#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarInputEvent.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarFont.hpp"
#include "StarBiMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
import star.text;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedMap.hpp"



import star.drawable;

import star.animation;


import star.interface_cursor;


import star.error_screen;
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
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.widget_parsing;
import star.gui_reader;
import star.pane;
import star.game_timers;
import star.pane_manager;
import star.label_widget;

namespace Star {

ErrorScreen::ErrorScreen() {
  m_paneManager = make_shared<PaneManager>();

  m_accepted = true;

  auto assets = Root::singleton().assets();

  m_guiContext = GuiContext::singletonPtr();

  m_errorPane = make_shared<Pane>();
  GuiReader reader;
  reader.registerCallback("btnOk", [this](Widget*) {
      m_accepted = true;
    });
  reader.construct(assets->json("/interface/windowconfig/error.config:paneLayout"), m_errorPane.get());
}

void ErrorScreen::setMessage(String const& errorMessage) {
  m_errorPane->fetchChild<LabelWidget>("labelError")->setText(errorMessage);
  m_accepted = false;

  if (!m_paneManager->isDisplayed(m_errorPane)) {
    m_paneManager->displayPane(PaneLayer::Window, m_errorPane, [this](PanePtr) {
      m_accepted = true;
    });
  }
}

bool ErrorScreen::accepted() {
  return m_accepted;
}

void ErrorScreen::render() {
  m_paneManager->render();
}

bool ErrorScreen::handleInputEvent(InputEvent const& event) {
  if (auto mouseMove = event.ptr<MouseMoveEvent>())
    m_cursorScreenPos = Vec2I(mouseMove->mousePosition);

  return m_paneManager->sendInputEvent(event);
}

void ErrorScreen::update(float dt) {
  m_paneManager->update(dt);
  m_cursor.update(dt);
}

void ErrorScreen::renderCursor() {
  Vec2I cursorPos = m_cursorScreenPos;
  Vec2I cursorSize = m_cursor.size();
  Vec2I cursorOffset = m_cursor.offset();
  float cursorScale = m_cursor.scale(interfaceScale());
  Drawable cursorDrawable = m_cursor.drawable();

  cursorPos[0] -= cursorOffset[0] * cursorScale;
  cursorPos[1] -= (cursorSize[1] - cursorOffset[1]) * cursorScale;
  if (!m_guiContext->trySetCursor(cursorDrawable, cursorOffset, cursorScale))
    m_guiContext->drawDrawable(cursorDrawable, Vec2F(cursorPos), cursorScale);
}

float ErrorScreen::interfaceScale() const {
  return m_guiContext->interfaceScale();
}

unsigned ErrorScreen::windowHeight() const {
  return m_guiContext->windowHeight();
}

unsigned ErrorScreen::windowWidth() const {
  return m_guiContext->windowWidth();
}

}
