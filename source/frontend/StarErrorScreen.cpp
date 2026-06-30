#include "StarErrorScreen.hpp"
#include "StarAlgorithm.hpp"
#include "StarGuiReader.hpp"
#include "StarJsonExtra.hpp"
#include "StarPaneManager.hpp"
#include "StarLabelWidget.hpp"
#include "StarAssets.hpp"
#include "StarGameTypes.hpp"

namespace Star {

ErrorScreen::ErrorScreen(ErrorScreenServices services)
  : m_guiContext(services.guiContext),
    m_assets(requireServiceValueAs<StarException>(std::move(services.assets), "ErrorScreen", "assets")),
    m_imageMetadata(requireServiceValueAs<StarException>(std::move(services.imageMetadata), "ErrorScreen", "image metadata")),
    m_cursor(InterfaceCursorServices{m_assets, m_imageMetadata}) {
  m_paneManager = make_shared<PaneManager>(m_guiContext);

  m_accepted = true;

  m_errorPane = make_shared<Pane>(m_guiContext);
  GuiReader reader(m_guiContext);
  reader.registerCallback("btnOk", [this](Widget*) {
      m_accepted = true;
    });
  reader.construct(m_assets->json("/interface/windowconfig/error.config:paneLayout"), m_errorPane.get());
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
  if (!m_guiContext.trySetCursor(cursorDrawable, cursorOffset, cursorScale))
    m_guiContext.drawDrawable(cursorDrawable, Vec2F(cursorPos), cursorScale);
}

float ErrorScreen::interfaceScale() const {
  return m_guiContext.interfaceScale();
}

unsigned ErrorScreen::windowHeight() const {
  return m_guiContext.windowHeight();
}

unsigned ErrorScreen::windowWidth() const {
  return m_guiContext.windowWidth();
}

}
