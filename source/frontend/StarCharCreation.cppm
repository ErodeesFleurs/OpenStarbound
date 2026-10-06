module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarImageProcessing.hpp"
#include "StarHumanoid.hpp"


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
import star.pane;

namespace Star {

class Player;
typedef shared_ptr<Player> PlayerPtr;

struct CharCreationExceptionTag {
  static constexpr char const* name() { return "CharCreationException"; }
};
using CharCreationException = StarError<CharCreationExceptionTag, StarException>;

STAR_CLASS(CharCreationPane);
class CharCreationPane : public Pane {
public:
  // The callback here is either called with null (when the user hits the
  // cancel button) or the newly created player (when the user hits the save
  // button).
  CharCreationPane(function<void(PlayerPtr)> requestCloseFunc);

  void randomize();
  void randomizeName();

  virtual void tick(float dt) override;
  virtual bool sendEvent(InputEvent const& event) override;

  virtual PanePtr createTooltip(Vec2I const&) override;

private:
  void nameBoxCallback(Widget* object);

  void changed();

  void createPlayer();

  PlayerPtr m_previewPlayer;

  StringList m_speciesList;

  size_t m_speciesChoice;
  size_t m_genderChoice;
  size_t m_modeChoice;
  size_t m_bodyColor;
  size_t m_alty;
  size_t m_hairChoice;
  size_t m_heady;
  size_t m_shirtChoice;
  size_t m_shirtColor;
  size_t m_pantsChoice;
  size_t m_pantsColor;
  size_t m_personality;
};

}

export module star.char_creation;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::CharCreationExceptionTag;
  using ::Star::CharCreationException;
  using ::Star::CharCreationPane;
  using ::Star::CharCreationPanePtr;
  using ::Star::CharCreationPaneConstPtr;
  using ::Star::CharCreationPaneWeakPtr;
  using ::Star::CharCreationPaneConstWeakPtr;
  using ::Star::CharCreationPaneUPtr;
  using ::Star::CharCreationPaneConstUPtr;
}
