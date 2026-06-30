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

template <typename T>
class WidgetRef {
  T* m_ptr = nullptr;

public:
  WidgetRef() = default;
  WidgetRef(std::nullptr_t) {}
  explicit WidgetRef(T& ref) : m_ptr(&ref) {}

  T* operator->() const { return m_ptr; }
  T& operator*() const { return *m_ptr; }
  T* get() const { return m_ptr; }
  T& ref() const { return *m_ptr; }

  explicit operator bool() const { return m_ptr != nullptr; }

  bool operator==(WidgetRef const& other) const { return m_ptr == other.m_ptr; }
  bool operator!=(WidgetRef const& other) const { return m_ptr != other.m_ptr; }
  bool operator==(std::nullptr_t) const { return m_ptr == nullptr; }
  bool operator!=(std::nullptr_t) const { return m_ptr != nullptr; }
};

template <typename Type1, typename Type2>
WidgetRef<Type1> as(WidgetRef<Type2> ref) {
  if (ref)
    return WidgetRef<Type1>(dynamic_cast<Type1&>(*ref));
  return nullptr;
}

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

  virtual void addChild(String const& name, UniquePtr<Widget> member);
  virtual void addChildAt(String const& name, UniquePtr<Widget> member, size_t at);
  [[nodiscard]] virtual bool removeChild(Widget& member);
  [[nodiscard]] virtual bool removeChild(String const& name);
  [[nodiscard]] virtual bool removeChildAt(size_t at);
  [[nodiscard]] virtual WidgetRef<Widget> getChildAt(Vec2I const& pos);
  [[nodiscard]] virtual bool containsChild(String const& name);
  [[nodiscard]] virtual WidgetRef<Widget> fetchChild(String const& name);
  template <typename WidgetType>
  [[nodiscard]] WidgetRef<WidgetType> fetchChild(String const& name);

  [[nodiscard]] virtual WidgetRef<Widget> findChild(String const& name);
  template <typename WidgetType>
  [[nodiscard]] WidgetRef<WidgetType> findChild(String const& name);

  [[nodiscard]] virtual size_t numChildren() const;
  [[nodiscard]] virtual WidgetRef<Widget> getChildNum(size_t num) const;
  template <typename WidgetType>
  [[nodiscard]] WidgetRef<WidgetType> getChildNum(size_t num) const;
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

  [[nodiscard]] virtual Widget* keyboardCapturer() const;
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
  List<UniquePtr<Widget>> m_members;
  StringMap<size_t> m_memberHash;
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
WidgetRef<WidgetType> Widget::getChildNum(size_t num) const {
  return as<WidgetType>(getChildNum(num));
}

template <typename WidgetType>
WidgetRef<WidgetType> Widget::fetchChild(String const& name) {
  return as<WidgetType>(fetchChild(name));
}

template <typename WidgetType>
WidgetRef<WidgetType> Widget::findChild(String const& name) {
  return as<WidgetType>(findChild(name));
}

}

template <> struct std::formatter<Star::Widget> : Star::OstreamFormatter {};
