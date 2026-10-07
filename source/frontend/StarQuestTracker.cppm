module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


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

STAR_CLASS(LabelWidget);
STAR_CLASS(ImageWidget);
STAR_CLASS(ImageStretchWidget);
STAR_CLASS(ProgressWidget);
STAR_CLASS(Quest);

class QuestTrackerPane : public Pane {
public:
  QuestTrackerPane();

  bool sendEvent(InputEvent const& event) override;
  void update(float dt) override;

  void setQuest(QuestPtr const& quest);

private:
  void setExpanded(bool expanded);

  ImageWidgetPtr m_frame;
  ImageStretchWidgetPtr m_expandedFrame;

  LabelWidgetPtr m_questObjectiveList;

  ImageWidgetPtr m_compassFrame;
  ImageWidgetPtr m_compass;

  ImageWidgetPtr m_progressFrame;
  ProgressWidgetPtr m_progress;

  int m_expandedFrameMinHeight;
  int m_expandedFramePadding;

  float m_compassDirection;
  float m_compassSpeed;
  float m_compassAcceleration;
  float m_compassFriction;

  QuestPtr m_currentQuest;
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
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::ImageWidget;
  using ::Star::ImageWidgetPtr;
  using ::Star::ImageWidgetConstPtr;
  using ::Star::ImageWidgetWeakPtr;
  using ::Star::ImageWidgetConstWeakPtr;
  using ::Star::ImageWidgetUPtr;
  using ::Star::ImageWidgetConstUPtr;
  using ::Star::ImageStretchWidget;
  using ::Star::ImageStretchWidgetPtr;
  using ::Star::ImageStretchWidgetConstPtr;
  using ::Star::ImageStretchWidgetWeakPtr;
  using ::Star::ImageStretchWidgetConstWeakPtr;
  using ::Star::ImageStretchWidgetUPtr;
  using ::Star::ImageStretchWidgetConstUPtr;
  using ::Star::ProgressWidget;
  using ::Star::ProgressWidgetPtr;
  using ::Star::ProgressWidgetConstPtr;
  using ::Star::ProgressWidgetWeakPtr;
  using ::Star::ProgressWidgetConstWeakPtr;
  using ::Star::ProgressWidgetUPtr;
  using ::Star::ProgressWidgetConstUPtr;
  using ::Star::Quest;
  using ::Star::QuestPtr;
  using ::Star::QuestConstPtr;
  using ::Star::QuestWeakPtr;
  using ::Star::QuestConstWeakPtr;
  using ::Star::QuestUPtr;
  using ::Star::QuestConstUPtr;
  using ::Star::QuestTrackerPane;
}
