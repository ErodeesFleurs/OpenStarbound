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
using PanePtr = std::shared_ptr<Pane>;
using PaneConstPtr = std::shared_ptr<Pane const>;
using PaneWeakPtr = std::weak_ptr<Pane>;
using PaneConstWeakPtr = std::weak_ptr<Pane const>;
using PaneUPtr = std::unique_ptr<Pane>;
using PaneConstUPtr = std::unique_ptr<Pane const>;
class PaneManager;
using PaneManagerPtr = std::shared_ptr<PaneManager>;
using PaneManagerConstPtr = std::shared_ptr<PaneManager const>;
using PaneManagerWeakPtr = std::weak_ptr<PaneManager>;
using PaneManagerConstWeakPtr = std::weak_ptr<PaneManager const>;
using PaneManagerUPtr = std::unique_ptr<PaneManager>;
using PaneManagerConstUPtr = std::unique_ptr<PaneManager const>;
class GuiContext;
using GuiContextPtr = std::shared_ptr<GuiContext>;
using GuiContextConstPtr = std::shared_ptr<GuiContext const>;
using GuiContextWeakPtr = std::weak_ptr<GuiContext>;
using GuiContextConstWeakPtr = std::weak_ptr<GuiContext const>;
using GuiContextUPtr = std::unique_ptr<GuiContext>;
using GuiContextConstUPtr = std::unique_ptr<GuiContext const>;

class ErrorScreen;
using ErrorScreenPtr = std::shared_ptr<ErrorScreen>;
using ErrorScreenConstPtr = std::shared_ptr<ErrorScreen const>;
using ErrorScreenWeakPtr = std::weak_ptr<ErrorScreen>;
using ErrorScreenConstWeakPtr = std::weak_ptr<ErrorScreen const>;
using ErrorScreenUPtr = std::unique_ptr<ErrorScreen>;
using ErrorScreenConstUPtr = std::unique_ptr<ErrorScreen const>;

class ErrorScreen {
public:
  ErrorScreen();

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
  PaneManagerPtr m_paneManager;
  PanePtr m_errorPane;

  bool m_accepted;
  Vec2I m_cursorScreenPos;
  InterfaceCursor m_cursor;
};

}

export module star.error_screen;

export namespace Star {
  using ::Star::Pane;
  using ::Star::PanePtr;
  using ::Star::PaneConstPtr;
  using ::Star::PaneWeakPtr;
  using ::Star::PaneConstWeakPtr;
  using ::Star::PaneUPtr;
  using ::Star::PaneConstUPtr;
  using ::Star::PaneManager;
  using ::Star::PaneManagerPtr;
  using ::Star::PaneManagerConstPtr;
  using ::Star::PaneManagerWeakPtr;
  using ::Star::PaneManagerConstWeakPtr;
  using ::Star::PaneManagerUPtr;
  using ::Star::PaneManagerConstUPtr;
  using ::Star::GuiContext;
  using ::Star::GuiContextPtr;
  using ::Star::GuiContextConstPtr;
  using ::Star::GuiContextWeakPtr;
  using ::Star::GuiContextConstWeakPtr;
  using ::Star::GuiContextUPtr;
  using ::Star::GuiContextConstUPtr;
  using ::Star::ErrorScreen;
  using ::Star::ErrorScreenPtr;
  using ::Star::ErrorScreenConstPtr;
  using ::Star::ErrorScreenWeakPtr;
  using ::Star::ErrorScreenConstWeakPtr;
  using ::Star::ErrorScreenUPtr;
  using ::Star::ErrorScreenConstUPtr;
}
