module;
import std;
import star.json;

import star.vector;
import star.string;
import star.input_event;

import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;





import star.drawable;
import star.animation;
import star.interface_cursor;

namespace Star {

class Pane;
class PaneManager;
class GuiContext;

class ErrorScreen {
public:
  ErrorScreen(GuiContext& context);

  // Resets accepted
  void setMessage(String const& message);

  bool accepted();

  void render();

  bool handleInputEvent(InputEvent const& event);
  void update(float dt);

private:
  void renderCursor();

  float interfaceScale() const;
  unsigned windowHeight() const;
  unsigned windowWidth() const;

  GuiContext* m_guiContext;
  std::shared_ptr<PaneManager> m_paneManager;
  std::shared_ptr<Pane> m_errorPane;

  bool m_accepted;
  Vec2I m_cursorScreenPos;
  InterfaceCursor m_cursor;
};

}

export module star.error_screen;

export namespace Star {
  using ::Star::Pane;
  using ::Star::PaneManager;
  using ::Star::GuiContext;
  using ::Star::ErrorScreen;
}
