module;
import std;
import star.string;
import star.json;

import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;

import star.vector;
namespace Star {

struct Drawable;

class Animation {
public:
  // config can be either a path to a config or a literal config.
  Animation();
  Animation(Json config, String const& directory = {});

  void setAngle(float angle);

  void setProcessing(DirectivesGroup processing);

  void setColor(Color color);

  void setTag(String tagName, String tagValue);
  void clearTags();

  Drawable drawable(float pixelSize) const;

  void update(float dt);

  bool isComplete() const;
  void reset();

private:
  enum AnimationMode { Stop, EndAndDisappear, LoopForever };
  static EnumMap<AnimationMode> AnimationModeNames;

  AnimationMode m_mode;
  String m_directory;
  String m_base;
  bool m_appendFrame;
  int m_frameNumber;
  float m_animationCycle;
  float m_animationTime;
  float m_angle;
  Vec2F m_offset;
  bool m_centered;
  DirectivesGroup m_processing;
  Color m_color;
  int m_variantOffset;

  StringMap<String> m_tagValues;
  int m_frame;
  float m_animationTimer;
  float m_timeToLive;
  bool m_completed;
};

}

export module star.animation;

export namespace Star {
  using ::Star::Drawable;
  using ::Star::Animation;
}
