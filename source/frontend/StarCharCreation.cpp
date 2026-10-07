#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarTtlCache.hpp"
#include "StarCasting.hpp"
#include "StarGameTypes.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarStrongTypedef.hpp"
#include "StarRpcPromise.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarDataStream.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarImageProcessing.hpp"
#include "StarRandom.hpp"
#include "StarLogging.hpp"
#include "StarOrderedMap.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarVersion.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarEither.hpp"
#include "StarWeightedPool.hpp"
#include "StarInterpolation.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"

#include "StarLuaRoot.hpp"

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;

#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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
import star.animation;
import star.particle;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;


import star.char_creation;
import star.widget_parsing;
import star.gui_reader;
import star.liquid_types;
import star.worker_pool;
import star.tile_sector_array;
import star.weather_types;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.button_group;
import star.button_widget;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.ai_types;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.portrait_widget;
import star.text_box_widget;
import star.label_widget;
import star.image_widget;
import star.status_effect_item;
import star.effect_source_item;
import star.previewable_item;
import star.tool_user_item;
import star.fireable_item;
import star.swingable_item;
import star.armors;
import star.player_log;
import star.name_generator;

import star.species_database;


import star.player_inventory;
import star.player_factory;

namespace Star {

CharCreationPane::CharCreationPane(std::function<void(PlayerPtr)> requestCloseFunc) {
  auto& root = Root::singleton();

  m_speciesList = jsonToStringList(root.assets()->json("/interface/windowconfig/charcreation.config:speciesOrdering"));

  GuiReader guiReader;
  guiReader.registerCallback("cancel", [=](Widget*) { requestCloseFunc({}); });
  guiReader.registerCallback("saveChar", [=, this](Widget*) {
      if (fetchChild<ButtonWidget>("btnSkipIntro")->isChecked())
        m_previewPlayer->log()->setIntroComplete(true);
      requestCloseFunc(m_previewPlayer);
      createPlayer();
      randomize();
      randomizeName();
    });

  guiReader.registerCallback("mainSkinColor.up", [this](Widget*) {
      m_bodyColor++;
      changed();
    });
  guiReader.registerCallback("mainSkinColor.down", [this](Widget*) {
      m_bodyColor--;
      changed();
    });
  guiReader.registerCallback("alty.up", [this](Widget*) {
      m_alty++;
      changed();
    });
  guiReader.registerCallback("alty.down", [this](Widget*) {
      m_alty--;
      changed();
    });
  guiReader.registerCallback("hairStyle.up", [this](Widget*) {
      m_hairChoice++;
      changed();
    });
  guiReader.registerCallback("hairStyle.down", [this](Widget*) {
      m_hairChoice--;
      changed();
    });
  guiReader.registerCallback("shirt.up", [this](Widget*) {
      m_shirtChoice++;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("shirt.down", [this](Widget*) {
      m_shirtChoice--;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("pants.up", [this](Widget*) {
      m_pantsChoice++;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("pants.down", [this](Widget*) {
      m_pantsChoice--;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("heady.up", [this](Widget*) {
      m_heady++;
      changed();
    });
  guiReader.registerCallback("heady.down", [this](Widget*) {
      m_heady--;
      changed();
    });
  guiReader.registerCallback("shirtColor.up", [this](Widget*) {
      m_shirtColor++;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("shirtColor.down", [this](Widget*) {
      m_shirtColor--;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("pantsColor.up", [this](Widget*) {
      m_pantsColor++;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("pantsColor.down", [this](Widget*) {
      m_pantsColor--;
      fetchChild<ButtonWidget>("btnToggleClothing")->setChecked(true);
      changed();
    });
  guiReader.registerCallback("personality.up", [this](Widget*) {
      m_personality++;
      changed();
    });
  guiReader.registerCallback("personality.down", [this](Widget*) {
      m_personality--;
      changed();
    });
  guiReader.registerCallback("toggleClothing", [this](Widget*) {
      changed();
    });

  guiReader.registerCallback("randomName", [this](Widget*) { randomizeName(); });
  guiReader.registerCallback("randomize", [this](Widget*) { randomize(); });

  guiReader.registerCallback("name", [this](Widget* object) { nameBoxCallback(object); });

  guiReader.registerCallback("species", [this](Widget* button) {
      size_t speciesChoice = convert<ButtonWidget>(button)->buttonGroupId();
      if (speciesChoice < m_speciesList.size() && speciesChoice != m_speciesChoice) {
        m_speciesChoice = speciesChoice;
        randomize();
        randomizeName();
      }
    });
  guiReader.registerCallback("gender", [this](Widget* button) {
      m_genderChoice = convert<ButtonWidget>(button)->buttonGroupId();
      changed();
    });

  guiReader.registerCallback("mode", [this](Widget* button) {
      m_modeChoice = convert<ButtonWidget>(button)->buttonGroupId();
      changed();
    });

  guiReader.construct(root.assets()->json("/interface/windowconfig/charcreation.config:paneLayout"), this);

  createPlayer();

  RandomSource random;
  m_speciesChoice = random.randu32() % m_speciesList.size();
  m_genderChoice = random.randu32();
  m_modeChoice = 1;
  randomize();
  randomizeName();
}

void CharCreationPane::createPlayer() {
  m_previewPlayer = Root::singleton().playerFactory()->create();
  try {
    auto portrait = fetchChild<PortraitWidget>("charPreview");
    if ((bool)portrait) {
      portrait->setEntity(m_previewPlayer);
    } else {
      throw CharCreationException("The charPreview portrait has the wrong type.");
    }
  } catch (CharCreationException const& e) {
    Logger::error("Character Preview portrait was not found in the json specification. {}", outputException(e, false));
  }
}

void CharCreationPane::randomize() {
  RandomSource random;
  m_bodyColor = random.randu32();
  m_hairChoice = random.randu32();
  m_alty = random.randu32();
  m_heady = random.randu32();
  m_shirtChoice = random.randu32();
  m_shirtColor = random.randu32();
  m_pantsChoice = random.randu32();
  m_pantsColor = random.randu32();
  m_personality = random.randu32();
  changed();
}

void CharCreationPane::tick(float dt) {
  Pane::tick(dt);
  if (!active())
    return;
  if (!m_previewPlayer)
    return;
  m_previewPlayer->animatePortrait(dt);
}

bool CharCreationPane::sendEvent(InputEvent const& event) {
  if (active() && m_previewPlayer) {
    if (event.is<KeyDownEvent>()) {
      auto actions = context()->actions(event);
      if (actions.contains(InterfaceAction::EmoteBlabbering))
        m_previewPlayer->addEmote(HumanoidEmote::Blabbering);
      if (actions.contains(InterfaceAction::EmoteShouting))
        m_previewPlayer->addEmote(HumanoidEmote::Shouting);
      if (actions.contains(InterfaceAction::EmoteHappy))
        m_previewPlayer->addEmote(HumanoidEmote::Happy);
      if (actions.contains(InterfaceAction::EmoteSad))
        m_previewPlayer->addEmote(HumanoidEmote::Sad);
      if (actions.contains(InterfaceAction::EmoteNeutral))
        m_previewPlayer->addEmote(HumanoidEmote::NEUTRAL);
      if (actions.contains(InterfaceAction::EmoteLaugh))
        m_previewPlayer->addEmote(HumanoidEmote::Laugh);
      if (actions.contains(InterfaceAction::EmoteAnnoyed))
        m_previewPlayer->addEmote(HumanoidEmote::Annoyed);
      if (actions.contains(InterfaceAction::EmoteOh))
        m_previewPlayer->addEmote(HumanoidEmote::Oh);
      if (actions.contains(InterfaceAction::EmoteOooh))
        m_previewPlayer->addEmote(HumanoidEmote::OOOH);
      if (actions.contains(InterfaceAction::EmoteBlink))
        m_previewPlayer->addEmote(HumanoidEmote::Blink);
      if (actions.contains(InterfaceAction::EmoteWink))
        m_previewPlayer->addEmote(HumanoidEmote::Wink);
      if (actions.contains(InterfaceAction::EmoteEat))
        m_previewPlayer->addEmote(HumanoidEmote::Eat);
      if (actions.contains(InterfaceAction::EmoteSleep))
        m_previewPlayer->addEmote(HumanoidEmote::Sleep);
    }
  }
  return Pane::sendEvent(event);
}

void CharCreationPane::randomizeName() {
  auto species = Root::singleton().speciesDatabase()->species(m_speciesList[m_speciesChoice]);
  auto tb = fetchChild<TextBoxWidget>("name");
  auto genderOption = species->options().genderOptions.wrap(m_genderChoice);
  int limiter = 100;
  while (!tb->setText(Root::singleton().nameGenerator()->generateName(species->nameGen(genderOption.gender)))) {
    if (limiter == 0)
      break;
    limiter--;
  }
  changed();
}

void CharCreationPane::changed() {
  auto& root = Root::singleton();

  auto textBox = fetchChild<TextBoxWidget>("name");
  auto speciesDefinition = Root::singleton().speciesDatabase()->species(m_speciesList[m_speciesChoice]);
  auto species = speciesDefinition->options();
  auto genderOptions = species.genderOptions.wrap(m_genderChoice);
  int genderIdx = pmod<int64_t>(m_genderChoice, species.genderOptions.size());

  auto labels = speciesDefinition->charGenTextLabels();

  fetchChild<LabelWidget>("labelMainSkinColor")->setText(labels[0]);
  fetchChild<LabelWidget>("labelHairStyle")->setText(labels[1]);
  fetchChild<LabelWidget>("labelShirt")->setText(labels[2]);
  fetchChild<LabelWidget>("labelPants")->setText(labels[3]);
  if (!labels[4].empty()) {
    fetchChild<LabelWidget>("labelAlty")->setText(labels[4]);
    fetchChild<LabelWidget>("labelAlty")->show();
    fetchChild<Widget>("alty")->show();
  } else {
    fetchChild<LabelWidget>("labelAlty")->hide();
    fetchChild<Widget>("alty")->hide();
  }
  fetchChild<LabelWidget>("labelHeady")->setText(labels[5]);
  fetchChild<LabelWidget>("labelShirtColor")->setText(labels[6]);
  fetchChild<LabelWidget>("labelPantsColor")->setText(labels[7]);
  fetchChild<LabelWidget>("labelPortrait")->setText(labels[8]);
  fetchChild<LabelWidget>("labelPersonality")->setText(labels[9]);

  if (auto speciesButton = fetchChild<ButtonWidget>(strf("species.{}", m_speciesChoice)))
    speciesButton->check();
  if (auto genderButton = fetchChild<ButtonWidget>(strf("gender.{}", genderIdx)))
    genderButton->check();

  auto modeButton = fetchChild<ButtonWidget>(strf("mode.{}", m_modeChoice));
  modeButton->check();
  setLabel("labelMode", modeButton->data().getString("description", "fail"));

  // Update the gender images for the new species
  for (size_t i = 0; i < species.genderOptions.size(); i++)
    if (auto button = fetchChild<ButtonWidget>(strf("gender.{}", i)))
      button->setOverlayImage(species.genderOptions[i].image);

  for (auto const& nameDefPair : root.speciesDatabase()->allSpecies()) {
    String name;
    SpeciesDefinitionPtr def;
    std::tie(name, def) = nameDefPair;
    // NOTE: Probably not hot enough to matter, but this contains and indexOf makes this loop
    // O(n^2).  This is less than ideal.
    if (m_speciesList.contains(name)) {
      if (auto bw = fetchChild<ButtonWidget>(strf("species.{}", m_speciesList.indexOf(name))))
        bw->setOverlayImage(def->options().genderOptions[genderIdx].characterImage);
    }
  }

  auto portrait = fetchChild<PortraitWidget>("charPreview");
  if (fetchChild<ButtonWidget>("btnToggleClothing")->isChecked())
    portrait->setMode(PortraitMode::Full);
  else
    portrait->setMode(PortraitMode::FullNude);

  auto results = root.speciesDatabase()->createHumanoid(
    textBox->getText(),
    species.species,
    m_genderChoice,
    m_bodyColor,
    m_alty,
    m_hairChoice,
    m_heady,
    m_shirtChoice,
    m_shirtColor,
    m_pantsChoice,
    m_pantsColor,
    m_personality
  );


  m_previewPlayer->setModeType((PlayerMode)m_modeChoice);

  m_previewPlayer->setHumanoidParameters(results.humanoidParameters);
  m_previewPlayer->setIdentity(results.identity);
  m_previewPlayer->refreshHumanoidParameters();
  for (auto p : EquipmentSlotNames) {
    if (auto equipment = results.armor.maybe(p.second)) {
      m_previewPlayer->inventory()->setItem(InventorySlot(p.first), root.itemDatabase()->item(ItemDescriptor(equipment.value())));
    } else {
      m_previewPlayer->inventory()->consumeSlot(InventorySlot(p.first));
    }
  }
  m_previewPlayer->refreshEquipment();

  m_previewPlayer->finalizeCreation();
}

void CharCreationPane::nameBoxCallback(Widget* object) {
  if (as<TextBoxWidget>(object))
    changed();
  else
    throw GuiException("Invalid object type, expected TextBoxWidget.");
}

PanePtr CharCreationPane::createTooltip(Vec2I const& screenPosition) {
  // what's under my cursor
  if (WidgetPtr child = getChildAt(screenPosition)) {
    // is it a species button ?
    if (child->parent()->name() == "species") {
      // which species is it ?
      size_t speciesIndex = convert<ButtonWidget>(child)->buttonGroupId();

      // no tooltips for unassigned button indices
      if (speciesIndex >= m_speciesList.size())
        return {};

      String speciesName = m_speciesList[speciesIndex];
      Star::SpeciesDefinitionPtr speciesDefinition = Root::singleton().speciesDatabase()->species(speciesName);

      // make a tooltip from the config file
      PanePtr tooltip = make_shared<Pane>();
      tooltip->removeAllChildren();
      GuiReader reader;
      auto& root = Root::singleton();
      String tooltipKind = "/interface/tooltips/species.tooltip";
      reader.construct(root.assets()->json(tooltipKind), tooltip.get());

      // find out the gender option block from the currently selected gender
      auto genderOption = speciesDefinition->options().genderOptions.wrap(m_genderChoice);
      // makes an icon out of the default gendered character image
      WidgetPtr titleIcon = make_shared<ImageWidget>(genderOption.characterImage);

      // read the description out of the already loaded species database.
      String title = speciesDefinition->tooltip().title;
      String subTitle = speciesDefinition->tooltip().subTitle;
      tooltip->setTitle(titleIcon, title, subTitle);

      tooltip->setLabel("descriptionLabel", speciesDefinition->tooltip().description);

      return tooltip;
    }
  }

  return {};
}

}
