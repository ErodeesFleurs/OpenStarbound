module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarLuaComponents.hpp"




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
import star.widget_parsing;
import star.gui_reader;
import star.base_script_pane;
import star.chat_types;

namespace Star {

STAR_CLASS(UniverseClient);
STAR_CLASS(TextBoxWidget);
STAR_CLASS(LabelWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(ImageStretchWidget);
STAR_CLASS(CanvasWidget);
STAR_CLASS(Chat);

class Chat : public BaseScriptPane {
public:
  Chat(UniverseClientPtr client, Json const&);

  void startChat();
  void startCommand();
  bool hasFocus() const override;
  virtual bool sendEvent(InputEvent const& event) override;
  void stopChat();
  virtual void renderImpl() override;
  virtual void hide() override;

  virtual void update(float dt) override;

  void addLine(String const& text, bool showPane = true);
  void addMessages(List<ChatReceivedMessage> const& messages, bool showPane = true);
  void addHistory(String const& chat);
  void clear(size_t count = std::numeric_limits<size_t>::max());

  String currentChat() const;
  bool setCurrentChat(String const& chat, bool moveCursor = false);
  void clearCurrentChat();

  ChatSendMode sendMode() const;

  void incrementIndex();
  void decrementIndex();

  float visible() const;

  void scrollUp();
  void scrollDown();
  void scrollBottom();

private:
  struct LogMessage {
    MessageContext::Mode mode;
    String portrait;
    String text;
  };

  void updateBottomButton();

  UniverseClientPtr m_client;
  bool m_scripted;

  TextBoxWidgetPtr m_textBox;
  LabelWidgetPtr m_say;
  ButtonWidgetPtr m_bottomButton;
  ButtonWidgetPtr m_upButton;
  Deque<String> m_chatHistory;
  unsigned m_chatPrevIndex;
  int64_t m_timeChatLastActive;
  float m_chatVisTime;
  float m_fadeRate;
  TextStyle m_chatTextStyle;
  unsigned m_chatHistoryLimit;
  int m_historyOffset;
  String m_chatFormatString;

  CanvasWidgetPtr m_chatLog;
  Vec2I m_chatLogPadding;

  ImageStretchWidgetPtr m_background;
  int m_defaultHeight;
  int m_bodyHeight;
  int m_expandedBodyHeight;
  bool m_expanded;

  void updateSize();

  Vec2I m_portraitTextOffset;
  Vec2I m_portraitImageOffset;
  float m_portraitScale;
  int m_portraitVerticalMargin;
  String m_portraitBackground;

  Map<MessageContext::Mode, String> m_colorCodes;
  Deque<LogMessage> m_receivedMessages;

  Set<MessageContext::Mode> m_modeFilter;
  ChatSendMode m_sendMode;
};

}

export module star.chat;

export namespace Star {
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
  using ::Star::TextBoxWidget;
  using ::Star::TextBoxWidgetPtr;
  using ::Star::TextBoxWidgetConstPtr;
  using ::Star::TextBoxWidgetWeakPtr;
  using ::Star::TextBoxWidgetConstWeakPtr;
  using ::Star::TextBoxWidgetUPtr;
  using ::Star::TextBoxWidgetConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::ImageStretchWidget;
  using ::Star::ImageStretchWidgetPtr;
  using ::Star::ImageStretchWidgetConstPtr;
  using ::Star::ImageStretchWidgetWeakPtr;
  using ::Star::ImageStretchWidgetConstWeakPtr;
  using ::Star::ImageStretchWidgetUPtr;
  using ::Star::ImageStretchWidgetConstUPtr;
  using ::Star::CanvasWidget;
  using ::Star::CanvasWidgetPtr;
  using ::Star::CanvasWidgetConstPtr;
  using ::Star::CanvasWidgetWeakPtr;
  using ::Star::CanvasWidgetConstWeakPtr;
  using ::Star::CanvasWidgetUPtr;
  using ::Star::CanvasWidgetConstUPtr;
  using ::Star::Chat;
  using ::Star::ChatPtr;
  using ::Star::ChatConstPtr;
  using ::Star::ChatWeakPtr;
  using ::Star::ChatConstWeakPtr;
  using ::Star::ChatUPtr;
  using ::Star::ChatConstUPtr;
}
