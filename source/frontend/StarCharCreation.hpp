#pragma once

#include "StarIAssets.hpp"
#include "StarPane.hpp"
#include "StarImageProcessing.hpp"
#include "StarHumanoid.hpp"

namespace Star {

class Player;
using PlayerPtr = shared_ptr<Player>;
class PlayerFactory;
using PlayerFactoryConstPtr = SharedPtr<PlayerFactory const>;
class SpeciesDatabase;
using SpeciesDatabaseConstPtr = SharedPtr<SpeciesDatabase const>;
class PatternedNameGenerator;
using PatternedNameGeneratorConstPtr = SharedPtr<PatternedNameGenerator const>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;

struct CharCreationServices {
  IAssetsConstPtr assets;
  PlayerFactoryConstPtr playerFactory;
  SpeciesDatabaseConstPtr speciesDatabase;
  PatternedNameGeneratorConstPtr nameGenerator;
  ItemDatabaseConstPtr itemDatabase;
};

struct CharCreationExceptionTag { static constexpr char const* typeName = "CharCreationException"; };
using CharCreationException = TypedException<StarException, CharCreationExceptionTag>;

class CharCreationPane : public Pane {
public:
  // The callback here is either called with null (when the user hits the
  // cancel button) or the newly created player (when the user hits the save
  // button).
  CharCreationPane(function<void(PlayerPtr)> requestCloseFunc,
      CharCreationServices services = {});

  void randomize();
  void randomizeName();

  virtual void tick(float dt) override;
  virtual bool sendEvent(InputEvent const& event) override;

  virtual PanePtr createTooltip(Vec2I const&) override;

private:
  void nameBoxCallback(Widget* object);

  void changed();

  void createPlayer();

  IAssetsConstPtr m_assets;
  PlayerFactoryConstPtr m_playerFactory;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  PatternedNameGeneratorConstPtr m_nameGenerator;
  ItemDatabaseConstPtr m_itemDatabase;

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
