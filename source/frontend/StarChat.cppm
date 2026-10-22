module;
import std;
import star.format;
import star.type_identity;
import star.json;

import star.vector;
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
import star.lua_components;




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
class TextBoxWidget;
class LabelWidget;
class ButtonWidget;
class ImageStretchWidget;
class CanvasWidget;

class Chat : public BaseScriptPane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<Chat, BaseScriptPane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<Chat>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<Chat>, StarException> create(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, Json const&);

  [[nodiscard]] std::expected<void, StarException> startChat();
  [[nodiscard]] std::expected<void, StarException> startCommand();
  [[nodiscard]] std::expected<bool, StarException> hasFocus() const override;
  [[nodiscard]] virtual std::expected<bool, StarException> sendEvent(InputEvent const& event) override;
  [[nodiscard]] std::expected<void, StarException> stopChat();
  [[nodiscard]] virtual std::expected<void, StarException> renderImpl() override;
  [[nodiscard]] virtual std::expected<void, StarException> hide() override;

  [[nodiscard]] virtual std::expected<void, StarException> update(float dt) override;

  [[nodiscard]] std::expected<void, StarException> addLine(String const& text, bool showPane = true);
  [[nodiscard]] std::expected<void, StarException> addMessages(List<ChatReceivedMessage> const& messages, bool showPane = true);
  [[nodiscard]] std::expected<void, StarException> addHistory(String const& chat);
  [[nodiscard]] std::expected<void, StarException> clear(std::size_t count = std::numeric_limits<std::size_t>::max());

  [[nodiscard]] std::expected<String, StarException> currentChat() const;
  [[nodiscard]] std::expected<bool, StarException> setCurrentChat(String const& chat, bool moveCursor = false);
  [[nodiscard]] std::expected<void, StarException> clearCurrentChat();

  [[nodiscard]] std::expected<ChatSendMode, StarException> sendMode() const;

  [[nodiscard]] std::expected<void, StarException> incrementIndex();
  [[nodiscard]] std::expected<void, StarException> decrementIndex();

  [[nodiscard]] std::expected<float, StarException> visible() const;

  [[nodiscard]] std::expected<void, StarException> scrollUp();
  [[nodiscard]] std::expected<void, StarException> scrollDown();
  [[nodiscard]] std::expected<void, StarException> scrollBottom();

private:
  Chat(GuiContext& guiContext, std::shared_ptr<UniverseClient> client);
  [[nodiscard]] std::expected<void, StarException> initialize(Json const& config);
  struct LogMessage {
    MessageContext::Mode mode;
    String portrait;
    String text;
  };

  [[nodiscard]] std::expected<void, StarException> updateBottomButton();

  std::shared_ptr<UniverseClient> m_client;
  bool m_scripted;

  std::shared_ptr<TextBoxWidget> m_textBox;
  std::shared_ptr<LabelWidget> m_say;
  std::shared_ptr<ButtonWidget> m_bottomButton;
  std::shared_ptr<ButtonWidget> m_upButton;
  Deque<String> m_chatHistory;
  unsigned m_chatPrevIndex;
  std::int64_t m_timeChatLastActive;
  float m_chatVisTime;
  float m_fadeRate;
  TextStyle m_chatTextStyle;
  unsigned m_chatHistoryLimit;
  int m_historyOffset;
  String m_chatFormatString;

  std::shared_ptr<CanvasWidget> m_chatLog;
  Vec2I m_chatLogPadding;

  std::shared_ptr<ImageStretchWidget> m_background;
  int m_defaultHeight;
  int m_bodyHeight;
  int m_expandedBodyHeight;
  bool m_expanded;

  [[nodiscard]] std::expected<void, StarException> updateSize();

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
  using ::Star::TextBoxWidget;
  using ::Star::LabelWidget;
  using ::Star::ButtonWidget;
  using ::Star::ImageStretchWidget;
  using ::Star::CanvasWidget;
  using ::Star::Chat;
}
