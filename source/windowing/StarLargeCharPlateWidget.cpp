#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarIdMap.hpp"
#include "StarArray.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarStrongTypedef.hpp"
#include "StarAStar.hpp"
#include "StarOrderedSet.hpp"
#include "StarEither.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"

import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
import star.renderer;
#include "StarLuaRoot.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.uuid;
import star.item_descriptor;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
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
import star.button_group;
import star.button_widget;
import star.portrait_widget;
import star.label_widget;
import star.large_char_plate_widget;

namespace Star {

LargeCharPlateWidget::LargeCharPlateWidget(WidgetCallbackFunc mainCallback, PlayerPtr player) : m_player(player) {
  m_portraitScale = 0;

  setSize(ButtonWidget::size());

  auto assets = Root::singleton().assets();
  m_config = assets->json("/interface.config:largeCharPlate");
  auto charPlateImage = m_config.getString("backingImage");

  setCallback(mainCallback);
  setImages(charPlateImage);

  m_playerPlate = charPlateImage;
  m_playerPlateHover = m_config.getString("playerHover");
  m_noPlayerPlate = m_config.getString("noPlayer");
  m_noPlayerPlateHover = m_config.getString("noPlayerHover");
  m_portraitOffset = jsonToVec2I(m_config.get("portraitOffset"));
  m_portraitScale = m_config.getFloat("portraitScale");

  String switchText = m_config.getString("switchText");
  String createText = m_config.getString("createText");

  m_portrait = make_shared<PortraitWidget>();
  m_portrait->setScale(m_portraitScale);
  m_portrait->setPosition(m_portraitOffset);
  m_portrait->setRenderHumanoid(true);
  addChild("portrait", m_portrait);

  String modeLabelText = m_config.getString("modeText");
  m_regularTextColor = Color::rgb(jsonToVec3B(m_config.get("textColor")));
  m_disabledTextColor = Color::rgb(jsonToVec3B(m_config.get("textColorDisabled")));

  m_modeNameOffset = jsonToVec2I(m_config.get("modeNameOffset"));
  m_modeOffset = jsonToVec2I(m_config.get("modeOffset"));

  auto modeNameHAnchor = HorizontalAnchorNames.getLeft(m_config.getString("modeNameHAnchor", "mid"));
  auto modeNameVAnchor = VerticalAnchorNames  .getLeft(m_config.getString("modeNameVAnchor", "bottom"));
  m_modeName = make_shared<LabelWidget>(modeLabelText, Color::White, modeNameHAnchor);
  addChild("modeName", m_modeName);
  m_modeName->setPosition(m_modeNameOffset);
  m_modeName->setAnchor(modeNameHAnchor, modeNameVAnchor);

  auto modeHAnchor = HorizontalAnchorNames.getLeft(m_config.getString("modeHAnchor", "left"));
  auto modeVAnchor = VerticalAnchorNames  .getLeft(m_config.getString("modeVAnchor", "bottom"));
  m_mode = make_shared<LabelWidget>();
  addChild("mode", m_mode);
  m_mode->setPosition(m_modeOffset);
  m_mode->setAnchor(modeHAnchor, modeVAnchor);

  m_createCharText = m_config.getString("noPlayerText");
  m_createCharTextColor = Color::rgb(jsonToVec3B(m_config.get("noPlayerTextColor")));
  m_playerNameOffset = jsonToVec2I(m_config.get("playerNameOffset"));

  auto playerNameHAnchor = HorizontalAnchorNames.getLeft(m_config.getString("playerNameHAnchor", "mid"));
  auto playerNameVAnchor = VerticalAnchorNames  .getLeft(m_config.getString("playerNameVAnchor", "bottom"));
  m_playerName = make_shared<LabelWidget>();
  m_playerName->setColor(m_createCharTextColor);
  m_playerName->setPosition(m_playerNameOffset);
  m_playerName->setAnchor(playerNameHAnchor, playerNameVAnchor);
  addChild("player", m_playerName);
}

void LargeCharPlateWidget::renderImpl() {
  Vec2I pressedOffset = isPressed() ? ButtonWidget::pressedOffset() : Vec2I(0, 0);

  m_portrait->setPosition(m_portraitOffset + pressedOffset);
  m_mode->setPosition(m_modeOffset + pressedOffset);
  m_modeName->setPosition(m_modeNameOffset + pressedOffset);
  m_playerName->setPosition(m_playerNameOffset + pressedOffset);
  if (m_delete) {
    m_delete->setPosition(m_deleteOffset + pressedOffset);
  }

  if (m_player) {
    ButtonWidget::setImages(m_playerPlate, m_playerPlateHover);
    ButtonWidget::renderImpl();
    m_modeName->setColor(m_regularTextColor);
    m_playerName->setColor(m_regularTextColor);
    m_playerName->setText(m_player->name());
  } else {
    ButtonWidget::setImages(m_noPlayerPlate, m_noPlayerPlateHover);
    ButtonWidget::enable();
    ButtonWidget::renderImpl();
    m_modeName->setColor(m_disabledTextColor);
    m_playerName->setColor(m_createCharTextColor);
    m_playerName->setText(m_createCharText);
  }
}

void LargeCharPlateWidget::mouseOut() {
  if (m_delete)
    m_delete->mouseOut();

  ButtonWidget::mouseOut();
}

void LargeCharPlateWidget::setPlayer(PlayerPtr player) {
  m_player = player;
  m_portrait->setEntity(m_player);

  if (m_player) {
    m_playerName->setText(m_player->name());
  } else {
    m_playerName->setText(m_createCharText);
  }

  auto modeTypeTextAndColor = Root::singleton().assets()->json("/interface.config:modeTypeTextAndColor").toArray();
  int modeType;
  if (m_player) {
    modeType = 1 + (int)m_player->modeType();
  } else {
    modeType = 0;
  }
  auto thisModeType = modeTypeTextAndColor[modeType].toArray();
  String modeTypeText = thisModeType[0].toString();
  Color modeTypeColor = Color::rgb(jsonToVec3B(thisModeType[1]));
  m_mode->setText(modeTypeText);
  m_mode->setColor(modeTypeColor);
}

void LargeCharPlateWidget::enableDelete(WidgetCallbackFunc const& callback) {
  disableDelete();

  auto trashButton = m_config.get("trashButton");
  auto baseImage = trashButton.getString("baseImage");
  auto hoverImage = trashButton.getString("hoverImage");
  auto pressedImage = trashButton.getString("pressedImage");
  auto disabledImage = trashButton.getString("disabledImage");
  auto offset = jsonToVec2I(trashButton.get("offset"));

  m_delete = make_shared<ButtonWidget>(callback, baseImage, hoverImage, pressedImage, disabledImage);
  addChild("trashButton", m_delete);
  m_delete->setPosition(offset);
  m_deleteOffset = offset;
}

void LargeCharPlateWidget::disableDelete() {
  if (m_delete) {
    removeChild(m_delete.get());
  }

  m_delete = {};
}

bool LargeCharPlateWidget::sendEvent(InputEvent const& event) {
  if (event.is<MouseMoveEvent>() && m_delete) {
    if (m_delete->inMember(*m_context->mousePosition(event)))
      m_delete->mouseOver();
    else
      m_delete->mouseOut();
  }

  if (Widget::sendEvent(event))
    return true;

  return ButtonWidget::sendEvent(event);
}

void LargeCharPlateWidget::update(float dt) {
  ButtonWidget::update(dt);

  if (!m_player || !m_config.getBool("animatePortrait", true))
    return;

  auto humanoid = m_player->humanoid();
  if (m_delete && m_delete->isHovered()) {
    humanoid->setEmoteState(HumanoidEmote::Sad);
    humanoid->setState(Humanoid::Run);
  } else {
    humanoid->setEmoteState(HumanoidEmote::Idle);
    humanoid->setState(isHovered() ? Humanoid::Walk : Humanoid::Idle);
  }
  humanoid->animate(dt, {});
}

}
