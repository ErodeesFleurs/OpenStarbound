#pragma once

#include "StarTime.hpp"
#include "StarRenderer.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
import star.font_texture_group;
#include "StarBiMap.hpp"
import star.anchor_types;
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
import star.text_painter;
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarBiMap.hpp"
import star.mixer;

namespace Star {

STAR_CLASS(Cinematic);
STAR_CLASS(Player);

class Cinematic {
public:
  Cinematic();

  void load(Json const& definition);

  void setPlayer(PlayerPtr player);

  void update(float dt);
  void render();

  bool completed() const;
  bool completable() const;

  // this won't synchronize audio, so it should only be used for testing
  void setTime(float timecode);

  void stop();

  bool handleInputEvent(InputEvent const& event);
  bool suppressInput() const;

  bool muteSfx() const;
  bool muteMusic() const;

private:
  struct TimeSkip {
    float availableTime;
    float skipToTime;
  };

  struct KeyFrame {
    float timecode;
    Json command;
  };

  struct CameraKeyFrame {
    float timecode;
    float zoom;
    Vec2F pan;
  };

  struct Panel {
    bool useCamera;
    String avatar;
    JsonArray drawables;
    int animationFrames;
    String text;
    TextPositioning textPosition;
    TextStyle textStyle;
    List<KeyFrame> keyFrames;
    float startTime;
    float endTime;
    float loopTime;
  };
  typedef shared_ptr<Panel> PanelPtr;

  struct PanelValues {
    float zoom;
    float alpha;
    Vec2F position;
    float frame;
    float textPercentage;
    bool completable;
  };

  struct AudioCue {
    AudioCue() : timecode(), endTimecode() {}
    String resource;
    int loops;
    float timecode;
    float endTimecode;
  };

  void drawDrawable(Drawable const& drawable, float drawableScale, Vec2F const& drawableTranslation);

  void updateCamera(float timecode);

  // since the clock time includes the background fade in/out time, this function gives the adjusted
  // timecode to use for events within the cinematic
  float currentTimecode() const;

  PanelValues determinePanelValues(PanelPtr panel, float timecode);

  List<TimeSkip> m_timeSkips;
  Maybe<TimeSkip> m_currentTimeSkip;

  List<CameraKeyFrame> m_cameraKeyFrames;
  List<PanelPtr> m_panels;
  List<AudioCue> m_audioCues;
  std::vector<AudioInstancePtr> m_activeAudio;

  // these include the time for background fades so they may not reflect the completion timecode
  Clock m_timer;
  float m_completionTime{};

  Maybe<Vec4B> m_backgroundColor;
  float m_backgroundFadeTime{};

  float m_cameraZoom{1.0f};
  Vec2F m_cameraPan{};

  float m_drawableScale{1.0f};
  Vec2F m_drawableTranslation{};
  Vec2F m_windowSize{};
  RectI m_scissorRect{};

  bool m_scissor{true};
  bool m_letterbox{true};

  PlayerPtr m_player;

  Vec2F m_offset{};

  bool m_skippable{true};
  bool m_suppressInput{false};

  bool m_muteSfx{false};
  bool m_muteMusic{false};

  bool m_completable{false};
};

}
