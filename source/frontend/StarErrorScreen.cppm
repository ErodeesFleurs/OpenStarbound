module;
import std;
import star.json;
import star.format;

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
  [[nodiscard]] static std::expected<std::shared_ptr<ErrorScreen>, StarException> create(GuiContext& context);

  // Resets accepted
  [[nodiscard]] std::expected<void, StarException> setMessage(String const& message);

  bool accepted();

  [[nodiscard]] std::expected<void, StarException> render();

  [[nodiscard]] std::expected<bool, StarException> handleInputEvent(InputEvent const& event);
  [[nodiscard]] std::expected<void, StarException> update(float dt);

private:
  explicit ErrorScreen(GuiContext& context);
  [[nodiscard]] std::expected<void, StarException> renderCursor();

  float interfaceScale() const;
  [[nodiscard]] std::expected<unsigned, StarException> windowHeight() const;
  [[nodiscard]] std::expected<unsigned, StarException> windowWidth() const;

  GuiContext* m_guiContext;
  std::shared_ptr<PaneManager> m_paneManager;
  std::shared_ptr<Pane> m_errorPane;

  bool m_accepted;
  Vec2I m_cursorScreenPos;
  std::shared_ptr<InterfaceCursor> m_cursor;
};

}

export module star.error_screen;

export namespace Star {
  using ::Star::Pane;
  using ::Star::PaneManager;
  using ::Star::GuiContext;
  using ::Star::ErrorScreen;
}
