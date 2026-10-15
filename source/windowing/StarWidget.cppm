module;
import std;
import star.format;
import star.stream_formatter;
import star.json;

import star.vector;
import star.casting;
import star.input_event;
import star.string;
import star.poly;
import star.maybe;
import star.list;
import star.map;

import star.rect;
namespace Star {

class GuiContext;

struct GuiExceptionTag {
  static constexpr char const* name() { return "GuiException"; }
};
using GuiException = StarError<GuiExceptionTag, StarException>;

class Widget;
class Pane;

enum class KeyboardCaptureMode {
  None,
  KeyEvents,
  TextInput
};

typedef std::function<void(Widget*)> WidgetCallbackFunc;

class Widget {
public:
  Widget();
  virtual ~Widget();

  Widget(Widget const& copy) = delete;
  Widget& operator=(Widget const&) = delete;

  virtual void render(RectI const& region) final;
  virtual void update(float dt);

  GuiContext* context() const;

  // Position of widget with drawing offset (useful for drawing)
  virtual Vec2I position() const;
  // Position of widget ignoring offset (useful for moving and placing widgets
  // relative to its current position)
  virtual Vec2I relativePosition() const;
  // Set position of widget, ignoring offset
  virtual void setPosition(Vec2I const& position);

  virtual Vec2I drawingOffset() const;
  virtual void setDrawingOffset(Vec2I const& offset);

  virtual Vec2I size() const;
  virtual void setSize(Vec2I const& size);

  virtual RectI relativeBoundRect() const;
  virtual RectI screenBoundRect() const;

  virtual bool inMember(Vec2I const& position) const;

  virtual bool sendEvent(InputEvent const& event);

  virtual void show();
  virtual void hide();
  virtual bool visibility() const;
  virtual void toggleVisibility();
  virtual void setVisibility(bool visibility);

  virtual void mouseOver();
  virtual void mouseOut();
  virtual void mouseReturnStillDown();
  virtual void setMouseTransparent(bool transparent);
  virtual bool mouseTransparent();

  virtual bool active() const;

  virtual bool interactive() const;

  virtual bool hasFocus() const;
  virtual void focus();
  virtual void blur();

  virtual Widget* parent() const;
  virtual void setParent(Widget* parent);

  virtual Pane const* window() const;
  virtual Pane* window();

  virtual void addChild(String const& name, std::shared_ptr<Widget> member);
  virtual void addChildAt(String const& name, std::shared_ptr<Widget> member, std::size_t at);
  virtual bool removeChild(Widget* member);
  virtual bool removeChild(String const& name);
  virtual bool removeChildAt(std::size_t at);
  virtual std::shared_ptr<Widget> getChildAt(Vec2I const& pos);
  virtual bool containsChild(String const& name);
  virtual std::shared_ptr<Widget> fetchChild(String const& name);
  template <typename WidgetType>
  std::shared_ptr<WidgetType> fetchChild(String const& name);

  virtual std::shared_ptr<Widget> findChild(String const& name);
  template <typename WidgetType>
  std::shared_ptr<WidgetType> findChild(String const& name);

  std::shared_ptr<Widget> childPtr(Widget const* widget) const;

  virtual std::size_t numChildren() const;
  virtual std::shared_ptr<Widget> getChildNum(std::size_t num) const;
  template <typename WidgetType>
  std::shared_ptr<WidgetType> getChildNum(std::size_t num) const;
  virtual void removeAllChildren();

  virtual String const& name() const;
  virtual void setName(String const& name);
  String fullName() const;

  unsigned windowHeight() const;
  unsigned windowWidth() const;
  Vec2I windowSize() const;
  virtual Vec2I screenPosition() const;
  void disableScissoring();
  void enableScissoring();
  void determineSizeFromChildren();
  void markAsContainer();

  virtual std::shared_ptr<Widget> keyboardCapturer() const;
  virtual KeyboardCaptureMode keyboardCaptureMode() const;
  virtual Maybe<std::pair<RectI, int>> keyboardCaptureArea() const;

  void setData(Json const& data);
  Json const& data();

  bool setLabel(String const& name, String const& value);

protected:
  friend std::ostream& operator<<(std::ostream& os, Widget const& widget);
  String toStringImpl(int indentLevel) const;

  virtual void renderImpl();
  virtual void drawChildren();
  bool setupDrawRegion(RectI const& region);
  virtual RectI getScissorRect() const;
  virtual RectI noScissor() const;

  Widget* m_parent;

  GuiContext* m_context;

  bool m_visible;
  PolyF m_boundPoly;

  Vec2I m_position;
  Vec2I m_size;
  RectI m_drawingArea;
  Vec2I m_drawingOffset;
  String m_name;
  List<std::shared_ptr<Widget>> m_members;
  StringMap<std::shared_ptr<Widget>> m_memberHash;
  Vec2I m_memberSize;
  bool m_focus;
  bool m_doScissor;
  bool m_container;
  bool m_mouseTransparent;

  Json m_data;
};

std::ostream& operator<<(std::ostream& os, Widget const& widget);

template <typename WidgetType>
std::shared_ptr<WidgetType> Widget::getChildNum(std::size_t num) const {
  return as<WidgetType>(getChildNum(num));
}

template <typename WidgetType>
std::shared_ptr<WidgetType> Widget::fetchChild(String const& name) {
  return as<WidgetType>(fetchChild(name));
}

template <typename WidgetType>
std::shared_ptr<WidgetType> Widget::findChild(String const& name) {
  return as<WidgetType>(findChild(name));
}

}

export module star.widget;

export template <> struct std::formatter<Star::Widget> : Star::StreamFormatter {};

export namespace Star {
  using ::Star::GuiExceptionTag;
  using ::Star::KeyboardCaptureMode;
  using ::Star::Widget;
  using ::Star::Pane;
  using ::Star::GuiException;
  using ::Star::WidgetCallbackFunc;
  using ::Star::GuiContext;
}
