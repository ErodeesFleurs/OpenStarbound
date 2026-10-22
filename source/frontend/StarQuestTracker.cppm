module;
import std;
import star.type_identity;
import star.format;
import star.json;

import star.casting;
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
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;


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

class LabelWidget;
class ImageWidget;
class ImageStretchWidget;
class ProgressWidget;
class Quest;

class QuestTrackerPane : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<QuestTrackerPane, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<QuestTrackerPane>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<QuestTrackerPane>, StarException> create(GuiContext& guiContext);

  [[nodiscard]] std::expected<bool, StarException> sendEvent(InputEvent const& event) override;
  [[nodiscard]] std::expected<void, StarException> update(float dt) override;

  void setQuest(std::shared_ptr<Quest> const& quest);

private:
  QuestTrackerPane(GuiContext& guiContext);
  [[nodiscard]] std::expected<void, StarException> initialize();
  void setExpanded(bool expanded);

  std::shared_ptr<ImageWidget> m_frame;
  std::shared_ptr<ImageStretchWidget> m_expandedFrame;

  std::shared_ptr<LabelWidget> m_questObjectiveList;

  std::shared_ptr<ImageWidget> m_compassFrame;
  std::shared_ptr<ImageWidget> m_compass;

  std::shared_ptr<ImageWidget> m_progressFrame;
  std::shared_ptr<ProgressWidget> m_progress;

  int m_expandedFrameMinHeight;
  int m_expandedFramePadding;

  float m_compassDirection;
  float m_compassSpeed;
  float m_compassAcceleration;
  float m_compassFriction;

  std::shared_ptr<Quest> m_currentQuest;
  bool m_expanded;

  String m_compassFrameImage;
  String m_expandedCompassFrameImage;

  String m_progressFrameImage;
  String m_expandedProgressFrameImage;

  String m_incompleteObjectiveTemplate;
  String m_completeObjectiveTemplate;
};

}

export module star.quest_tracker;

export namespace Star {
  using ::Star::LabelWidget;
  using ::Star::ImageWidget;
  using ::Star::ImageStretchWidget;
  using ::Star::ProgressWidget;
  using ::Star::Quest;
  using ::Star::QuestTrackerPane;
}
