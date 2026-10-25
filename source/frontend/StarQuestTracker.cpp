import std;
import star.json;
import star.format;
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
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.math_common;
import star.logging;


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


import star.quest_tracker;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;
import star.image_widget;
import star.image_stretch_widget;
import star.progress_widget;
import star.layout;
import star.vertical_layout;

import star.quests;

namespace Star {

QuestTrackerPane::QuestTrackerPane(GuiContext& guiContext) : Pane(guiContext) {}

std::expected<std::shared_ptr<QuestTrackerPane>, StarException> QuestTrackerPane::create(GuiContext& guiContext) {
  auto pane = std::shared_ptr<QuestTrackerPane>(new QuestTrackerPane(guiContext));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> QuestTrackerPane::initialize() {
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface/questtracker/questtracker.config");
  if (!config) return std::unexpected(std::move(config.error()));

  auto progressFrameImage = config->getString("progressFrameImage");
  if (!progressFrameImage) return std::unexpected(std::move(progressFrameImage.error()));
  m_progressFrameImage = std::move(*progressFrameImage);
  auto expandedProgressFrameImage = config->getString("expandedProgressFrameImage");
  if (!expandedProgressFrameImage) return std::unexpected(std::move(expandedProgressFrameImage.error()));
  m_expandedProgressFrameImage = std::move(*expandedProgressFrameImage);
  auto compassFrameImage = config->getString("compassFrameImage");
  if (!compassFrameImage) return std::unexpected(std::move(compassFrameImage.error()));
  m_compassFrameImage = std::move(*compassFrameImage);
  auto expandedCompassFrameImage = config->getString("expandedCompassFrameImage");
  if (!expandedCompassFrameImage) return std::unexpected(std::move(expandedCompassFrameImage.error()));
  m_expandedCompassFrameImage = std::move(*expandedCompassFrameImage);
  auto incompleteObjectiveTemplate = config->getString("incompleteObjectiveTemplate");
  if (!incompleteObjectiveTemplate) return std::unexpected(std::move(incompleteObjectiveTemplate.error()));
  m_incompleteObjectiveTemplate = std::move(*incompleteObjectiveTemplate);
  auto completeObjectiveTemplate = config->getString("completeObjectiveTemplate");
  if (!completeObjectiveTemplate) return std::unexpected(std::move(completeObjectiveTemplate.error()));
  m_completeObjectiveTemplate = std::move(*completeObjectiveTemplate);
  auto expandedFrameMinHeight = config->getInt("expandedFrameMinHeight");
  if (!expandedFrameMinHeight) return std::unexpected(std::move(expandedFrameMinHeight.error()));
  m_expandedFrameMinHeight = *expandedFrameMinHeight;
  auto expandedFramePadding = config->getInt("expandedFramePadding");
  if (!expandedFramePadding) return std::unexpected(std::move(expandedFramePadding.error()));
  m_expandedFramePadding = *expandedFramePadding;

  m_compassDirection = 0;
  m_compassSpeed = 0;
  auto compassAcceleration = config->getFloat("compassAcceleration");
  if (!compassAcceleration) return std::unexpected(std::move(compassAcceleration.error()));
  m_compassAcceleration = *compassAcceleration;
  auto compassFriction = config->getFloat("compassFriction");
  if (!compassFriction) return std::unexpected(std::move(compassFriction.error()));
  m_compassFriction = *compassFriction;

  GuiReader reader(context());
  auto layout = config->get("paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto constructed = reader.construct(*layout, *this);
  if (!constructed) return std::unexpected(std::move(constructed.error()));

  auto frame = fetchChild<ImageWidget>("imgFrame");
  if (!frame) return std::unexpected(std::move(frame.error()));
  m_frame = std::move(*frame);
  auto expandedFrame = fetchChild<ImageStretchWidget>("imgFrameExpanded");
  if (!expandedFrame) return std::unexpected(std::move(expandedFrame.error()));
  m_expandedFrame = std::move(*expandedFrame);
  auto compassFrame = fetchChild<ImageWidget>("imgCompassFrame");
  if (!compassFrame) return std::unexpected(std::move(compassFrame.error()));
  m_compassFrame = std::move(*compassFrame);
  auto compass = fetchChild<ImageWidget>("imgCompass");
  if (!compass) return std::unexpected(std::move(compass.error()));
  m_compass = std::move(*compass);
  auto questObjectiveList = fetchChild<LabelWidget>("lblQuestObjectiveList");
  if (!questObjectiveList) return std::unexpected(std::move(questObjectiveList.error()));
  m_questObjectiveList = std::move(*questObjectiveList);
  auto progress = fetchChild<ProgressWidget>("questProgress");
  if (!progress) return std::unexpected(std::move(progress.error()));
  m_progress = std::move(*progress);
  auto progressFrame = fetchChild<ImageWidget>("imgProgressFrame");
  if (!progressFrame) return std::unexpected(std::move(progressFrame.error()));
  m_progressFrame = std::move(*progressFrame);
  setExpanded(false);

  // allow things like the compass and progress bar to be drawn outside the background
  disableScissoring();
  m_progressFrame->disableScissoring();
  return {};
}

std::expected<bool, StarException> QuestTrackerPane::sendEvent(InputEvent const& event) {
  auto consumed = Pane::sendEvent(event);
  if (!consumed) return std::unexpected(std::move(consumed.error()));
  if (*consumed)
    return true;

  if (std::holds_alternative<MouseButtonDownEvent>(event) && std::get<MouseButtonDownEvent>(event).mouseButton == MouseButton::Left) {
    auto mousePos = *context().mousePosition(event);
    if ((m_expanded && m_expandedFrame->inMember(mousePos)) ||
        (!m_expanded && m_frame->inMember(mousePos))) {
      setExpanded(!m_expanded);
      return true;
    }
  }

  return false;
}

std::expected<void, StarException> QuestTrackerPane::update(float dt) {
  if (m_currentQuest) {
    if (auto objectiveList = m_currentQuest->objectiveList()) {
      if (objectiveList->size() == 0) {
        auto hidden = m_questObjectiveList->hide();
        if (!hidden) return std::unexpected(std::move(hidden.error()));
      } else {
        auto shown = m_questObjectiveList->show();
        if (!shown) return std::unexpected(std::move(shown.error()));
        if (m_expanded) {
          String listText = "";
          for (auto const& objective : *objectiveList) {
            auto completeJson = objective.get(1);
            if (!completeJson) return std::unexpected(std::move(completeJson.error()));
            auto complete = completeJson->toBool();
            if (!complete) return std::unexpected(std::move(complete.error()));
            auto textJson = objective.get(0);
            if (!textJson) return std::unexpected(std::move(textJson.error()));
            auto text = textJson->toString();
            if (!text) return std::unexpected(std::move(text.error()));
            listText += (*complete ? m_completeObjectiveTemplate : m_incompleteObjectiveTemplate)
                .replaceTags(StringMap<String>{{"objective", *text}});
          }
          auto updated = m_questObjectiveList->setText(listText);
          if (!updated) return std::unexpected(std::move(updated.error()));
        } else {
          String displayObjective = "";
          for (auto const& objective : *objectiveList) {
            auto completeJson = objective.get(1);
            if (!completeJson) return std::unexpected(std::move(completeJson.error()));
            auto complete = completeJson->toBool();
            if (!complete) return std::unexpected(std::move(complete.error()));
            if (displayObjective.empty() || !*complete) {
              auto textJson = objective.get(0);
              if (!textJson) return std::unexpected(std::move(textJson.error()));
              auto text = textJson->toString();
              if (!text) return std::unexpected(std::move(text.error()));
              displayObjective = (*complete ? m_completeObjectiveTemplate : m_incompleteObjectiveTemplate)
                  .replaceTags(StringMap<String>{{"objective", *text}});
              if (!*complete) break;
            }
          }
          auto updated = m_questObjectiveList->setText(displayObjective);
          if (!updated) return std::unexpected(std::move(updated.error()));
        }
      }
    } else {
      auto hidden = m_questObjectiveList->hide();
      if (!hidden) return std::unexpected(std::move(hidden.error()));
      auto cleared = m_questObjectiveList->setText("");
      if (!cleared) return std::unexpected(std::move(cleared.error()));
    }

    if (m_expanded) {
      auto shown = m_expandedFrame->show();
      if (!shown) return std::unexpected(std::move(shown.error()));
      auto hidden = m_frame->hide();
      if (!hidden) return std::unexpected(std::move(hidden.error()));

      int frameHeight = std::max(m_expandedFrameMinHeight, m_questObjectiveList->size()[1] + m_expandedFramePadding * 2);
      m_expandedFrame->setSize(Vec2I(m_expandedFrame->size()[0], frameHeight));
      m_expandedFrame->setPosition(Vec2I(m_expandedFrame->relativePosition()[0], -frameHeight));
    } else {
      auto shown = m_frame->show();
      if (!shown) return std::unexpected(std::move(shown.error()));
      auto hidden = m_expandedFrame->hide();
      if (!hidden) return std::unexpected(std::move(hidden.error()));
    }

    if (auto progress = m_currentQuest->progress()) {
      auto shown = m_progress->show();
      if (!shown) return std::unexpected(std::move(shown.error()));
      auto frameShown = m_progressFrame->show();
      if (!frameShown) return std::unexpected(std::move(frameShown.error()));

      if (m_expanded) {
        auto image = m_progressFrame->setImage(m_expandedProgressFrameImage);
        if (!image) return std::unexpected(std::move(image.error()));

        m_progress->setDrawingOffset(Vec2I(0, -m_expandedFrame->size()[1]));
        m_progressFrame->setDrawingOffset(Vec2I(0, -m_expandedFrame->size()[1]));
      } else {
        auto image = m_progressFrame->setImage(m_progressFrameImage);
        if (!image) return std::unexpected(std::move(image.error()));

        m_progress->setDrawingOffset(Vec2I(0, -m_frame->size()[1]));
        m_progressFrame->setDrawingOffset(Vec2I(0, -m_frame->size()[1]));
      }

      m_progress->setCurrentProgressLevel(*progress);
    } else {
      auto hidden = m_progress->hide();
      if (!hidden) return std::unexpected(std::move(hidden.error()));
      auto frameHidden = m_progressFrame->hide();
      if (!frameHidden) return std::unexpected(std::move(frameHidden.error()));
    }

    if (auto compassDirection = m_currentQuest->compassDirection()) {
      auto shown = m_compass->show();
      if (!shown) return std::unexpected(std::move(shown.error()));
      auto frameShown = m_compassFrame->show();
      if (!frameShown) return std::unexpected(std::move(frameShown.error()));

      auto image = m_compassFrame->setImage((m_expanded || m_currentQuest->progress())
          ? m_expandedCompassFrameImage : m_compassFrameImage);
      if (!image) return std::unexpected(std::move(image.error()));

      float compassDiff = pfmod(*compassDirection - m_compassDirection, (float)std::numbers::pi * 2);
      if (std::abs(compassDiff) < m_compassAcceleration && std::abs(m_compassSpeed) < m_compassAcceleration) {
        m_compassSpeed = 0;
        m_compassDirection = *compassDirection;
      } else {
        if (std::abs(compassDiff) > std::numbers::pi)
          compassDiff -= std::copysign(2 * std::numbers::pi, compassDiff);
        float diffRatio = std::abs(compassDiff) / std::numbers::pi;
        float speedChange = m_compassAcceleration * diffRatio;

        m_compassSpeed += std::copysign(speedChange, compassDiff);
        m_compassSpeed -= m_compassSpeed * m_compassFriction;
        m_compassDirection += m_compassSpeed;
      }

      auto rotated = m_compass->setRotation(m_compassDirection);
      if (!rotated) return std::unexpected(std::move(rotated.error()));
    } else {
      auto hidden = m_compass->hide();
      if (!hidden) return std::unexpected(std::move(hidden.error()));
      auto frameHidden = m_compassFrame->hide();
      if (!frameHidden) return std::unexpected(std::move(frameHidden.error()));
    }
  }

  return Pane::update(dt);
}

void QuestTrackerPane::setQuest(std::shared_ptr<Quest> const& quest) {
  m_currentQuest = quest;
}

void QuestTrackerPane::setExpanded(bool expanded) {
  m_expanded = expanded;
}

}
