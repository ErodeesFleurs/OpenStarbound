import std;
import star.json;
import star.json_extra;
import star.game_types;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.format;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.ordered_set;
import star.strong_typedef;
import star.logging;
import star.lexical_cast;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.renderer;
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
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;

import star.radio_message_database;

import star.game_timers;


import star.radio_message_popup;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;
import star.image_widget;

import star.math_common;
import star.format;
namespace Star {

RadioMessagePopup::RadioMessagePopup(GuiContext& guiContext) : Pane(guiContext), m_popupStage(PopupStage::Hidden) {}

std::expected<std::shared_ptr<RadioMessagePopup>, StarException> RadioMessagePopup::create(GuiContext& guiContext) {
  auto pane = std::shared_ptr<RadioMessagePopup>(new RadioMessagePopup(guiContext));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> RadioMessagePopup::initialize() {
  auto assets = context()->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface/radiomessage/radiomessage.config");
  if (!config) return std::unexpected(std::move(config.error()));
  auto layout = config->get("paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  GuiReader reader(*context());
  auto constructed = reader.construct(*layout, this);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  auto messageLabel = fetchChild<LabelWidget>("lblMessage");
  if (!messageLabel) return std::unexpected(std::move(messageLabel.error()));
  m_messageLabel = std::move(*messageLabel);
  auto portraitImage = fetchChild<ImageWidget>("imgPortrait");
  if (!portraitImage) return std::unexpected(std::move(portraitImage.error()));
  m_portraitImage = std::move(*portraitImage);
  auto backgroundImage = config->getString("backgroundImage");
  if (!backgroundImage) return std::unexpected(std::move(backgroundImage.error()));
  m_backgroundImage = std::move(*backgroundImage);
  auto animateInTime = config->getFloat("animateInTime");
  if (!animateInTime) return std::unexpected(std::move(animateInTime.error()));
  m_animateInTime = *animateInTime;
  auto animateInImage = config->getString("animateInImage");
  if (!animateInImage) return std::unexpected(std::move(animateInImage.error()));
  m_animateInImage = std::move(*animateInImage);
  auto animateInFrames = config->getInt("animateInFrames");
  if (!animateInFrames) return std::unexpected(std::move(animateInFrames.error()));
  m_animateInFrames = *animateInFrames;
  auto animateOutTime = config->getFloat("animateOutTime");
  if (!animateOutTime) return std::unexpected(std::move(animateOutTime.error()));
  m_animateOutTime = *animateOutTime;
  auto animateOutImage = config->getString("animateOutImage");
  if (!animateOutImage) return std::unexpected(std::move(animateOutImage.error()));
  m_animateOutImage = std::move(*animateOutImage);
  auto animateOutFrames = config->getInt("animateOutFrames");
  if (!animateOutFrames) return std::unexpected(std::move(animateOutFrames.error()));
  m_animateOutFrames = *animateOutFrames;
  auto chatOffsetJson = config->get("chatOffset");
  if (!chatOffsetJson) return std::unexpected(std::move(chatOffsetJson.error()));
  auto chatOffset = jsonToVec2I(*chatOffsetJson);
  if (!chatOffset) return std::unexpected(std::move(chatOffset.error()));
  m_chatOffset = *chatOffset;
  m_chatStartPosition = m_chatOffset;
  m_chatEndPosition = m_chatOffset;
  auto slideTime = config->getFloat("slideTime");
  if (!slideTime) return std::unexpected(std::move(slideTime.error()));
  m_slideTime = *slideTime;
  m_slideTimer = m_slideTime;
  updateAnchorOffset();
  return enterStage(PopupStage::Hidden);
}

std::expected<void, StarException> RadioMessagePopup::update(float dt) {
  if (messageActive()) {
    if (m_stageTimer.tick(dt)) {
      auto advanced = nextPopupStage();
      if (!advanced) return std::unexpected(std::move(advanced.error()));
    }

    if (m_popupStage == PopupStage::AnimateIn) {
      int frame = std::floor((1.0f - m_stageTimer.percent()) * m_animateInFrames);
      auto background = setBG("", strf("{}:{}", m_animateInImage, frame), "");
      if (!background) return std::unexpected(std::move(background.error()));
    } else if (m_popupStage == PopupStage::ScrollText) {
      int frame =
          int((m_stageTimer.timer / m_message.portraitSpeed) * m_message.portraitFrames) % m_message.portraitFrames;
      auto portraitPath = m_message.portraitImage.replace("<frame>", toString(frame));
      if (!portraitPath) return std::unexpected(std::move(portraitPath.error()));
      auto portrait = m_portraitImage->setImage(*portraitPath);
      if (!portrait) return std::unexpected(std::move(portrait.error()));
      auto length = Text::stripEscapeCodes(m_message.text).length();
      if (!length) return std::unexpected(std::move(length.error()));
      int textLength = std::floor(*length * (1.0f - m_stageTimer.percent()));
      auto limit = m_messageLabel->setTextCharLimit(textLength);
      if (!limit) return std::unexpected(std::move(limit.error()));
    } else if (m_popupStage == PopupStage::Persist) {
      // you're cool, just stay cool, cool person
    } else if (m_popupStage == PopupStage::AnimateOut) {
      int frame = std::floor((1.0f - m_stageTimer.percent()) * m_animateOutFrames);
      auto background = setBG("", strf("{}:{}", m_animateOutImage, frame), "");
      if (!background) return std::unexpected(std::move(background.error()));
    }

    m_slideTimer = std::min(m_slideTimer + dt, m_slideTime);
    updateAnchorOffset();
  }

  return Pane::update(dt);
}

std::expected<void, StarException> RadioMessagePopup::dismissed() {
  if (m_chatterSound)
    m_chatterSound->stop();
  return Pane::dismissed();
}

bool RadioMessagePopup::messageActive() {
  return m_popupStage != PopupStage::Hidden;
}

std::expected<void, StarException> RadioMessagePopup::setMessage(RadioMessage message) {
  m_message = message;
  if (!message.chatterSound.empty() && message.textSpeed > 0) {
    if (m_chatterSound)
      m_chatterSound->stop();
    auto assets = context()->root().assets();
    if (!assets) return std::unexpected(std::move(assets.error()));
    auto audio = (*assets)->audio(message.chatterSound);
    if (!audio) return std::unexpected(std::move(audio.error()));
    auto sound = AudioInstance::create(**audio);
    if (!sound) return std::unexpected(std::move(sound.error()));
    m_chatterSound = std::move(*sound);
    m_chatterSound->setLoops(-1);
  }
  auto entered = enterStage(PopupStage::AnimateIn);
  if (!entered) return std::unexpected(std::move(entered.error()));
  updateAnchorOffset();
  return {};
}

void RadioMessagePopup::setChatHeight(int chatHeight) {
  auto endPosition = m_chatOffset + Vec2I(0, chatHeight);
  if (endPosition != m_chatEndPosition) {
    m_chatStartPosition = anchorOffset();
    m_chatEndPosition = endPosition;
    m_slideTimer = 0.0f;
  }
}

std::expected<void, StarException> RadioMessagePopup::interrupt() {
  if (m_popupStage != PopupStage::Hidden && m_popupStage != PopupStage::AnimateOut)
    return enterStage(PopupStage::AnimateOut);
  return {};
}

void RadioMessagePopup::updateAnchorOffset() {
  float slideRatio = m_slideTimer / m_slideTime;
  setAnchorOffset(Vec2I(int(m_chatStartPosition[0] * (1 - slideRatio) + m_chatEndPosition[0] * slideRatio),
      int(m_chatStartPosition[1] * (1 - slideRatio) + m_chatEndPosition[1] * slideRatio)));
}

std::expected<void, StarException> RadioMessagePopup::nextPopupStage() {
  if (m_popupStage != PopupStage::Hidden)
    return enterStage((PopupStage)((int)m_popupStage + 1));
  return {};
}

std::expected<void, StarException> RadioMessagePopup::enterStage(PopupStage newStage) {
  m_popupStage = newStage;
  if (m_popupStage == PopupStage::Hidden) {
    auto portrait = m_portraitImage->hide();
    if (!portrait) return std::unexpected(std::move(portrait.error()));
    auto label = m_messageLabel->hide();
    if (!label) return std::unexpected(std::move(label.error()));
    return setBG("", strf("{}:0", m_animateInImage), "");
  } else if (m_popupStage == PopupStage::AnimateIn) {
    m_stageTimer = GameTimer(m_animateInTime);
    auto portrait = m_portraitImage->hide();
    if (!portrait) return std::unexpected(std::move(portrait.error()));
    return m_messageLabel->hide();
  } else if (m_popupStage == PopupStage::ScrollText) {
    if (m_message.textSpeed == 0) {
      // skip this stage if text is instant
      return enterStage(PopupStage::Persist);
    } else {
      auto length = Text::stripEscapeCodes(m_message.text).length();
      if (!length) return std::unexpected(std::move(length.error()));
      m_stageTimer = GameTimer(*length / m_message.textSpeed);
      auto portrait = m_portraitImage->show();
      if (!portrait) return std::unexpected(std::move(portrait.error()));
      auto label = m_messageLabel->show();
      if (!label) return std::unexpected(std::move(label.error()));
      auto text = m_messageLabel->setText(m_message.text);
      if (!text) return std::unexpected(std::move(text.error()));
      auto limit = m_messageLabel->setTextCharLimit(0);
      if (!limit) return std::unexpected(std::move(limit.error()));
      auto background = setBG(m_backgroundImage);
      if (!background) return std::unexpected(std::move(background.error()));
      if (m_chatterSound) {
        auto audio = context()->playAudio(m_chatterSound);
        if (!audio) return std::unexpected(std::move(audio.error()));
      }
    }
  } else if (m_popupStage == PopupStage::Persist) {
    m_stageTimer = GameTimer(m_message.persistTime);
    auto portrait = m_portraitImage->show();
    if (!portrait) return std::unexpected(std::move(portrait.error()));
    auto portraitPath = m_message.portraitImage.replace("<frame>", "0");
    if (!portraitPath) return std::unexpected(std::move(portraitPath.error()));
    auto image = m_portraitImage->setImage(*portraitPath);
    if (!image) return std::unexpected(std::move(image.error()));
    auto label = m_messageLabel->show();
    if (!label) return std::unexpected(std::move(label.error()));
    auto text = m_messageLabel->setText(m_message.text);
    if (!text) return std::unexpected(std::move(text.error()));
    auto limit = m_messageLabel->setTextCharLimit({});
    if (!limit) return std::unexpected(std::move(limit.error()));
    auto background = setBG(m_backgroundImage);
    if (!background) return std::unexpected(std::move(background.error()));
    if (m_chatterSound)
      m_chatterSound->stop();
  } else if (m_popupStage == PopupStage::AnimateOut) {
    m_stageTimer = GameTimer(m_animateOutTime);
    auto portrait = m_portraitImage->hide();
    if (!portrait) return std::unexpected(std::move(portrait.error()));
    return m_messageLabel->hide();
  }
  return {};
}

}
