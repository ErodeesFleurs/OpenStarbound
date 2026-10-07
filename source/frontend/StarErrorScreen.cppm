module;

#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarInputEvent.hpp"

#include "StarJson.hpp"
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





import star.drawable;
import star.animation;
import star.interface_cursor;

namespace Star {

STAR_CLASS(Pane);
STAR_CLASS(PaneManager);
STAR_CLASS(GuiContext);

STAR_CLASS(ErrorScreen);

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
