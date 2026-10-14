module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
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
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
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

class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<UniverseClient const>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<UniverseClient const>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<UniverseClient const>;
class TextBoxWidget;
using TextBoxWidgetPtr = std::shared_ptr<TextBoxWidget>;
using TextBoxWidgetConstPtr = std::shared_ptr<TextBoxWidget const>;
using TextBoxWidgetWeakPtr = std::weak_ptr<TextBoxWidget>;
using TextBoxWidgetConstWeakPtr = std::weak_ptr<TextBoxWidget const>;
using TextBoxWidgetUPtr = std::unique_ptr<TextBoxWidget>;
using TextBoxWidgetConstUPtr = std::unique_ptr<TextBoxWidget const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class ImageStretchWidget;
using ImageStretchWidgetPtr = std::shared_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstPtr = std::shared_ptr<ImageStretchWidget const>;
using ImageStretchWidgetWeakPtr = std::weak_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstWeakPtr = std::weak_ptr<ImageStretchWidget const>;
using ImageStretchWidgetUPtr = std::unique_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstUPtr = std::unique_ptr<ImageStretchWidget const>;
class CanvasWidget;
using CanvasWidgetPtr = std::shared_ptr<CanvasWidget>;
using CanvasWidgetConstPtr = std::shared_ptr<CanvasWidget const>;
using CanvasWidgetWeakPtr = std::weak_ptr<CanvasWidget>;
using CanvasWidgetConstWeakPtr = std::weak_ptr<CanvasWidget const>;
using CanvasWidgetUPtr = std::unique_ptr<CanvasWidget>;
using CanvasWidgetConstUPtr = std::unique_ptr<CanvasWidget const>;
class Chat;
using ChatPtr = std::shared_ptr<Chat>;
using ChatConstPtr = std::shared_ptr<Chat const>;
using ChatWeakPtr = std::weak_ptr<Chat>;
using ChatConstWeakPtr = std::weak_ptr<Chat const>;
using ChatUPtr = std::unique_ptr<Chat>;
using ChatConstUPtr = std::unique_ptr<Chat const>;

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
  void clear(std::size_t count = std::numeric_limits<std::size_t>::max());

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
  std::int64_t m_timeChatLastActive;
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
