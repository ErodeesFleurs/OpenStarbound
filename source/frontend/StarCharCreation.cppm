module;
#include "StarConfig.hpp"
import star.format;
import star.json;
import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.random;
import star.periodic_function;
import star.ordered_map;


import star.animation;
import star.particle;
import star.animated_part_set;
import star.light_source;
import star.networked_animator;
import star.humanoid;


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
