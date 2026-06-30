#pragma once

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarGuiContext.hpp"
#include "StarText.hpp"

namespace Star {

struct GuiExceptionTag { static constexpr char const* typeName = "GuiException"; };
using GuiException = TypedException<StarException, GuiExceptionTag>;

class Widget;
using WidgetPtr = SharedPtr<Widget>;
class Pane;
using PanePtr = SharedPtr<Pane>;

enum class KeyboardCaptureMode {
  None,
  KeyEvents,
  TextInput
};

using WidgetCallbackFunc = function<void(Widget*)>;

class Widget {
public:
  explicit Widget(GuiContext& context);
  virtual ~Widget();

  Widget(Widget const& copy) = delete;
  Widget& operator=(Widget const&) = delete;

  virtual void render(RectI const& region) final;
  virtual void update(float dt);

  [[nodiscard]] GuiContext& context() const;
  void setContext(GuiContext& context);

  // Position of widget with drawing offset (useful for drawing)
  [[nodiscard]] virtual Vec2I position() const;
  // Position of widget ignoring offset (useful for moving and placing widgets
  // relative to its current position)
  [[nodiscard]] virtual Vec2I relativePosition() const;
  // Set position of widget, ignoring offset
  virtual void setPosition(Vec2I const& position);

  [[nodiscard]] virtual Vec2I drawingOffset() const;
  virtual void setDrawingOffset(Vec2I const& offset);

  [[nodiscard]] virtual Vec2I size() const;
  virtual void setSize(Vec2I const& size);

  [[nodiscard]] virtual RectI relativeBoundRect() const;
  [[nodiscard]] virtual RectI screenBoundRect() const;

  [[nodiscard]] virtual bool inMember(Vec2I const& position) const;

  [[nodiscard]] virtual bool sendEvent(InputEvent const& event);

  virtual void show();
  virtual void hide();
  [[nodiscard]] virtual bool visibility() const;
  virtual void toggleVisibility();
  virtual void setVisibility(bool visibility);

  virtual void mouseOver();
  virtual void mouseOut();
  virtual void mouseReturnStillDown();
  virtual void setMouseTransparent(bool transparent);
  [[nodiscard]] virtual bool mouseTransparent();

  [[nodiscard]] virtual bool active() const;

  [[nodiscard]] virtual bool interactive() const;

  [[nodiscard]] virtual bool hasFocus() const;
  virtual void focus();
  virtual void blur();

  [[nodiscard]] virtual Widget* parent() const;
  virtual void setParent(Widget* parent);

  [[nodiscard]] virtual Pane const* window() const;
  [[nodiscard]] virtual Pane* window();

  virtual void addChild(String const& name, WidgetPtr member);
  virtual void addChildAt(String const& name, WidgetPtr member, size_t at);
  [[nodiscard]] virtual bool removeChild(Widget* member);
  [[nodiscard]] virtual bool removeChild(String const& name);
  [[nodiscard]] virtual bool removeChildAt(size_t at);
  [[nodiscard]] virtual WidgetPtr getChildAt(Vec2I const& pos);
  [[nodiscard]] virtual bool containsChild(String const& name);
  [[nodiscard]] virtual WidgetPtr fetchChild(String const& name);
  template <typename WidgetType>
  [[nodiscard]] shared_ptr<WidgetType> fetchChild(String const& name);

  [[nodiscard]] virtual WidgetPtr findChild(String const& name);
  template <typename WidgetType>
  [[nodiscard]] shared_ptr<WidgetType> findChild(String const& name);

  [[nodiscard]] WidgetPtr childPtr(Widget const* widget) const;

  [[nodiscard]] virtual size_t numChildren() const;
  [[nodiscard]] virtual WidgetPtr getChildNum(size_t num) const;
  template <typename WidgetType>
  [[nodiscard]] shared_ptr<WidgetType> getChildNum(size_t num) const;
  virtual void removeAllChildren();

  [[nodiscard]] virtual String const& name() const;
  virtual void setName(String const& name);
  [[nodiscard]] String fullName() const;

  [[nodiscard]] unsigned windowHeight() const;
  [[nodiscard]] unsigned windowWidth() const;
  [[nodiscard]] Vec2I windowSize() const;
  [[nodiscard]] virtual Vec2I screenPosition() const;
  void disableScissoring();
  void enableScissoring();
  void determineSizeFromChildren();
  void markAsContainer();

  [[nodiscard]] virtual WidgetPtr keyboardCapturer() const;
  [[nodiscard]] virtual KeyboardCaptureMode keyboardCaptureMode() const;
  [[nodiscard]] virtual Maybe<pair<RectI, int>> keyboardCaptureArea() const;

  void setData(Json const& data);
  [[nodiscard]] Json const& data();

  [[nodiscard]] bool setLabel(String const& name, String const& value);

protected:
  friend std::ostream& operator<<(std::ostream& os, Widget const& widget);
  [[nodiscard]] String toStringImpl(int indentLevel) const;

  virtual void renderImpl();
  virtual void drawChildren();
  [[nodiscard]] bool setupDrawRegion(RectI const& region);
  [[nodiscard]] virtual RectI getScissorRect() const;
  [[nodiscard]] virtual RectI noScissor() const;

  Widget* m_parent = nullptr;

  bool m_visible = true;
  PolyF m_boundPoly;

  Vec2I m_position;
  Vec2I m_size;
  RectI m_drawingArea;
  Vec2I m_drawingOffset;
  String m_name;
  List<WidgetPtr> m_members;
  StringMap<WidgetPtr> m_memberHash;
  Vec2I m_memberSize;
  bool m_focus = false;
  bool m_doScissor = true;
  bool m_container = false;
  bool m_mouseTransparent = false;

  Json m_data;

private:
  GuiContext* m_context = nullptr;
};

std::ostream& operator<<(std::ostream& os, Widget const& widget);

template <typename WidgetType>
shared_ptr<WidgetType> Widget::getChildNum(size_t num) const {
  return as<WidgetType>(getChildNum(num));
}

template <typename WidgetType>
shared_ptr<WidgetType> Widget::fetchChild(String const& name) {
  return as<WidgetType>(fetchChild(name));
}

template <typename WidgetType>
shared_ptr<WidgetType> Widget::findChild(String const& name) {
  return as<WidgetType>(findChild(name));
}

}

template <> struct std::formatter<Star::Widget> : Star::OstreamFormatter {};
