#pragma once

#include "StarDrawable.hpp"
#include "StarBiMap.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

class Animation;
using AnimationPtr = SharedPtr<Animation>;

class Animation {
public:
  Animation();
  // config can be either a path to a config or a literal config.
  Animation(Json config, String const& directory, AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase = {});

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

  AnimationMode m_mode = EndAndDisappear;
  String m_directory;
  String m_base;
  bool m_appendFrame = false;
  int m_frameNumber = 1;
  float m_animationCycle = 1.0f;
  float m_animationTime = 1.0f;
  float m_angle = 0.0f;
  Vec2F m_offset;
  bool m_centered = true;
  DirectivesGroup m_processing;
  Color m_color = Color::White;
  int m_variantOffset = 0;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;

  StringMap<String> m_tagValues;
  int m_frame = 0;
  float m_animationTimer = 0.0f;
  float m_timeToLive = 0.0f;
  bool m_completed = false;
};

}
