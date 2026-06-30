#pragma once

#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarInterfaceCursor.hpp"
#include "StarInputEvent.hpp"

namespace Star {

class Pane;
using PanePtr = SharedPtr<Pane>;
class PaneManager;
using PaneManagerPtr = SharedPtr<PaneManager>;
class GuiContext;

class ErrorScreen;
using ErrorScreenPtr = SharedPtr<ErrorScreen>;

struct ErrorScreenServices {
  AssetsConstPtr assets;
  ImageMetadataDatabaseConstPtr imageMetadata;
  GuiContext& guiContext;
};

class ErrorScreen {
public:
  explicit ErrorScreen(ErrorScreenServices services);

  // Resets accepted
  void setMessage(String const& message);

  [[nodiscard]] bool accepted();

  void render();

  [[nodiscard]] bool handleInputEvent(InputEvent const& event);
  void update(float dt);

private:
  void renderCursor();

  [[nodiscard]] float interfaceScale() const;
  [[nodiscard]] unsigned windowHeight() const;
  [[nodiscard]] unsigned windowWidth() const;

  GuiContext& m_guiContext;
  PaneManagerPtr m_paneManager;
  PanePtr m_errorPane;
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadata;

  bool m_accepted = true;
  Vec2I m_cursorScreenPos;
  InterfaceCursor m_cursor;
};

}
