module;

#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"

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
#include "StarVector.hpp"
import star.text;

#include "StarRect.hpp"
#include "StarList.hpp"





import star.world_geometry;
import star.chat_action;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.chat_bubble_separation;
import star.world_camera;

namespace Star {

STAR_CLASS(GuiContext);
STAR_CLASS(AssetTextureGroup);
STAR_CLASS(WorldClient);
STAR_CLASS(ChatBubbleManager);
STAR_CLASS(StoredFunction);

class ChatBubbleManager {
public:
  ChatBubbleManager();

  void setCamera(WorldCamera const& camera);

  void addChatActions(List<ChatAction> chatActions, bool silent = false);

  void update(float dt, WorldClientPtr world);
  void render();

private:
  typedef tuple<String, Vec2F> BubbleImage;
  typedef tuple<String, TextStyle, bool, Vec2F> BubbleText;

  struct Bubble {
    EntityId entity;
    String text;
    Json config;
    float age;
    List<BubbleImage> backgroundImages;
    List<BubbleText> bubbleText;
    bool onscreen;
  };

  struct PortraitBubble {
    EntityId entity;
    String portrait;
    String text;
    Vec2F position;
    Json config;
    float age;
    List<BubbleImage> backgroundImages;
    List<BubbleText> bubbleText;
    bool onscreen;
  };

  // Calculate the alpha for a speech bubble based on distance from player to
  // edge of screen
  uint8_t calcDistanceFadeAlpha(Vec2F bubbleScreenPosition, StoredFunctionPtr fadeFunction) const;

  RectF bubbleImageRect(Vec2F screenPos, BubbleImage const& bubbleImage, float pixelRatio);
  void drawBubbleImage(Vec2F screenPos, BubbleImage const& bubbleImage, float pixelRatio, int alpha);
  void drawBubbleText(Vec2F screenPos, BubbleText const& bubbleText, float pixelRatio, int alpha, bool isPortrait);

  GuiContext* m_guiContext;

  WorldCamera m_camera;

  TextPositioning m_textTemplate;
  TextPositioning m_portraitTextTemplate;
  TextStyle m_textStyle;
  Vec2F m_textPadding;

  BubbleSeparator<Bubble> m_bubbles;
  int m_zoom;
  float m_cachedInterfaceScale;
  Vec2F m_bubbleOffset;
  float m_maxAge;
  float m_portraitMaxAge;
  float m_interBubbleMargin;
  int m_maxMessagePerEntity;

  Deque<PortraitBubble> m_portraitBubbles;
  String m_portraitBackgroundImage;
  String m_portraitMoreImage;
  Vec2I m_portraitMorePosition;
  Vec2I m_portraitBackgroundSize;
  Vec2I m_portraitPosition;
  Vec2I m_portraitSize;
  Vec2I m_portraitTextPosition;
  unsigned m_portraitTextWidth;
  float m_portraitChatterFramerate;
  float m_portraitChatterDuration;

  float m_furthestVisibleTextDistance; // 0.0 is directly over the player, 1.0
  // is the edge of the window
  StoredFunctionPtr m_textFadeFunction;
  StoredFunctionPtr m_bubbleFadeFunction;
};

}

export module star.chat_bubble_manager;

export namespace Star {
  using ::Star::GuiContext;
  using ::Star::GuiContextPtr;
  using ::Star::GuiContextConstPtr;
  using ::Star::GuiContextWeakPtr;
  using ::Star::GuiContextConstWeakPtr;
  using ::Star::GuiContextUPtr;
  using ::Star::GuiContextConstUPtr;
  using ::Star::AssetTextureGroup;
  using ::Star::AssetTextureGroupPtr;
  using ::Star::AssetTextureGroupConstPtr;
  using ::Star::AssetTextureGroupWeakPtr;
  using ::Star::AssetTextureGroupConstWeakPtr;
  using ::Star::AssetTextureGroupUPtr;
  using ::Star::AssetTextureGroupConstUPtr;
  using ::Star::WorldClient;
  using ::Star::WorldClientPtr;
  using ::Star::WorldClientConstPtr;
  using ::Star::WorldClientWeakPtr;
  using ::Star::WorldClientConstWeakPtr;
  using ::Star::WorldClientUPtr;
  using ::Star::WorldClientConstUPtr;
  using ::Star::ChatBubbleManager;
  using ::Star::ChatBubbleManagerPtr;
  using ::Star::ChatBubbleManagerConstPtr;
  using ::Star::ChatBubbleManagerWeakPtr;
  using ::Star::ChatBubbleManagerConstWeakPtr;
  using ::Star::ChatBubbleManagerUPtr;
  using ::Star::ChatBubbleManagerConstUPtr;
  using ::Star::StoredFunction;
  using ::Star::StoredFunctionPtr;
  using ::Star::StoredFunctionConstPtr;
  using ::Star::StoredFunctionWeakPtr;
  using ::Star::StoredFunctionConstWeakPtr;
  using ::Star::StoredFunctionUPtr;
  using ::Star::StoredFunctionConstUPtr;
}
