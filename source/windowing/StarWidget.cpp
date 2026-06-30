#include "StarWidget.hpp"
#include "StarPane.hpp"

#include "StarLabelWidget.hpp"
#include "StarFlowLayout.hpp"

namespace Star {

Widget::Widget(GuiContext& context) {
  m_context.reset(&context);
}

Widget::~Widget() {
  removeAllChildren();
}

void Widget::update(float dt) {
  for (auto& widget : m_members)
    widget->update(dt);
}

GuiContext& Widget::context() const {
  return *m_context;
}

void Widget::setContext(GuiContext& context) {
  m_context.reset(&context);
  for (auto& child : m_members)
    child->setContext(context);
}

void Widget::render(RectI const& region) {
  if (!m_visible)
    return;

  if (!setupDrawRegion(region))
    return;

  renderImpl();
  drawChildren();
}

void Widget::renderImpl() {}

void Widget::drawChildren() {
  for (auto& child : m_members)
    child->render(m_drawingArea);
}

Vec2I Widget::position() const {
  return m_position + m_drawingOffset;
}

Vec2I Widget::relativePosition() const {
  return m_position;
}

bool Widget::setupDrawRegion(RectI const& region) {
  RectI scissorRect;
  if (m_doScissor) {
    scissorRect = getScissorRect();
  } else {
    scissorRect = noScissor();
  }
  m_drawingArea = scissorRect.limited(region);
  if (m_drawingArea.isEmpty())
    return false;

  context().setInterfaceScissorRect(m_drawingArea);
  return true;
}

Vec2I Widget::drawingOffset() const {
  return m_drawingOffset;
}

void Widget::setDrawingOffset(Vec2I const& offset) {
  m_drawingOffset = offset;
}

RectI Widget::getScissorRect() const {
  return screenBoundRect();
}

RectI Widget::noScissor() const {
  return RectI::inf();
}

void Widget::disableScissoring() {
  m_doScissor = false;
}

void Widget::enableScissoring() {
  m_doScissor = true;
}

void Widget::setPosition(Vec2I const& position) {
  m_position = position;
}

Vec2I Widget::size() const {
  return m_size;
}

void Widget::setSize(Vec2I const& size) {
  m_size = size;
}

RectI Widget::relativeBoundRect() const {
  return RectI::withSize(relativePosition(), size());
}

RectI Widget::screenBoundRect() const {
  return relativeBoundRect().translated(screenPosition() - relativePosition());
}

void Widget::determineSizeFromChildren() {
  Vec2I max;

  for (auto& child : m_members) {
    Vec2I childMax = child->position() + child->size();
    max = max.piecewiseMax(childMax);
  }

  setSize(max);
}

void Widget::markAsContainer() {
  m_container = true;
}

Widget* Widget::keyboardCapturer() const {
  if (active()) {
    for (auto const& member : m_members) {
      auto mode = member->keyboardCaptureMode();
      if (mode != KeyboardCaptureMode::None)
        return member.get();
      else if (auto capturer = member->keyboardCapturer())
        return capturer;
    }
  }
  return nullptr;
}

KeyboardCaptureMode Widget::keyboardCaptureMode() const {
  return KeyboardCaptureMode::None;
}

Maybe<pair<RectI, int>> Widget::keyboardCaptureArea() const {
  return {};
}

void Widget::setData(Json const& data) {
  m_data = data;
}

Json const& Widget::data() {
  return m_data;
}

Vec2I Widget::screenPosition() const {
  if (m_parent) {
    return m_parent->screenPosition() + position();
  } else {
    return position();
  }
}

bool Widget::inMember(Vec2I const& position) const {
  if (!m_visible)
    return false;

  if (m_mouseTransparent)
    return false;

  if (!m_drawingArea.isNull() && !m_drawingArea.contains(Vec2I::floor(position)))
    return false;

  if (m_container) {
    for (auto& child : m_members)
      if (child->inMember(position))
        return true;
  } else {
    return screenBoundRect().contains(position);
  }

  return false;
}

bool Widget::sendEvent(InputEvent const& event) {
  if (!m_visible)
    return false;

  for (auto& child : reverseIterate(m_members)) {
    if (child->sendEvent(event))
      return true;
  }

  return false;
}

void Widget::mouseOver() {}

void Widget::mouseOut() {}

void Widget::mouseReturnStillDown() {}

void Widget::setMouseTransparent(bool transparent) {
  m_mouseTransparent = transparent;
}

bool Widget::mouseTransparent() {
  return m_mouseTransparent;
}

observer_ptr<Widget> Widget::parent() const {
  return m_parent;
}

void Widget::setParent(observer_ptr<Widget> parent) {
  m_parent = parent;
}

void Widget::show() {
  m_visible = true;
}

void Widget::hide() {
  m_visible = false;
}

bool Widget::visibility() const {
  return m_visible;
}

void Widget::toggleVisibility() {
  m_visible = !m_visible;
}

void Widget::setVisibility(bool visibility) {
  if (visibility)
    show();
  else
    hide();
}

bool Widget::active() const {
  return m_visible;
}

bool Widget::interactive() const {
  return true;
}

bool Widget::hasFocus() const {
  return m_focus;
}

void Widget::focus() {
  m_focus = true;
  if (auto w = window())
    w->setFocus(*this);
}

void Widget::blur() {
  m_focus = false;
  if (window())
    window()->removeFocus(this);
}

unsigned Widget::windowHeight() const {
  return context().windowHeight();
}

unsigned Widget::windowWidth() const {
  return context().windowWidth();
}

Vec2I Widget::windowSize() const {
  return Vec2I(context().windowSize());
}

Pane* Widget::window() {
  if (m_parent)
    return m_parent->window();
  return nullptr;
}

Pane const* Widget::window() const {
  if (m_parent)
    return m_parent->window();
  return nullptr;
}

void Widget::addChild(String const& name, UniquePtr<Widget> member) {
  member->setName(name);
  member->setContext(*m_context);
  size_t index = m_members.size();
  m_members.push_back(std::move(member));
  m_memberHash[name] = index;
  m_members[index]->setParent(observer_ptr<Widget>(this));
}

void Widget::addChildAt(String const& name, UniquePtr<Widget> member, size_t at) {
  if (at > m_members.size())
    throw GuiException("Attempted to insert item after the end of the list.");

  m_members.insert(m_members.begin() + at, std::move(member));
  m_members[at]->setName(name);
  m_members[at]->setContext(*m_context);
  m_members[at]->setParent(observer_ptr<Widget>(this));
  // Rebuild hash from the insertion point
  for (size_t i = at; i < m_members.size(); ++i)
    m_memberHash[m_members[i]->name()] = i;
}

bool Widget::removeChild(Widget& member) {
  for (size_t i = 0; i < m_members.size(); ++i) {
    if (m_members[i].get() == &member) {
      m_memberHash.erase(member.name());
      m_members[i]->setParent(nullptr);
      m_members.erase(m_members.begin() + i);
      return true;
    }
    if (m_members[i]->removeChild(member))
      return true;
  }
  return false;
}

bool Widget::removeChild(String const& name) {
  if (name.contains(".")) {
    StringList nameList = name.split(".", 1);
    auto it = m_memberHash.find(nameList[0]);
    if (it != m_memberHash.end())
      return m_members[it->second]->removeChild(nameList[1]);
    return false;
  }

  auto it = m_memberHash.find(name);
  if (it == m_memberHash.end())
    return false;

  size_t index = it->second;
  m_memberHash.erase(it);
  m_members[index]->setParent(nullptr);
  m_members.erase(m_members.begin() + index);

  // Update hash entries for shifted indices
  for (size_t i = index; i < m_members.size(); ++i)
    m_memberHash[m_members[i]->name()] = i;

  return true;
}

bool Widget::removeChildAt(size_t at) {
  if (at >= m_members.size())
    return false;

  m_memberHash.erase(m_members[at]->name());
  m_members[at]->setParent(nullptr);
  m_members.erase(m_members.begin() + at);

  for (size_t i = at; i < m_members.size(); ++i)
    m_memberHash[m_members[i]->name()] = i;

  return true;
}

void Widget::removeAllChildren() {
  for (auto& child : m_members)
    child->setParent(nullptr);

  m_members.clear();
  m_memberHash.clear();
}

bool Widget::containsChild(String const& name) {
  return fetchChild(name).operator bool();
}

WidgetRef<Widget> Widget::fetchChild(String const& name) {
  if (name.contains(".")) {
    StringList nameList = name.split(".", 1);
    auto it = m_memberHash.find(nameList[0]);
    if (it != m_memberHash.end())
      return m_members[it->second]->fetchChild(nameList[1]);
  } else {
    auto it = m_memberHash.find(name);
    if (it != m_memberHash.end())
      return WidgetRef<Widget>(*m_members[it->second]);
  }
  return nullptr;
}

WidgetRef<Widget> Widget::findChild(String const& name) {
  if (auto found = fetchChild(name))
    return found;
  for (auto const& child : m_members) {
    if (auto found = child->findChild(name))
      return found;
  }
  return nullptr;
}

WidgetRef<Widget> Widget::getChildAt(Vec2I const& pos) {
  for (auto& child : reverseIterate(m_members)) {
    if (child->inMember(pos)) {
      auto res = child->getChildAt(pos);
      if (res)
        return res;
      return WidgetRef<Widget>(*child);
    }
  }
  return nullptr;
}

size_t Widget::numChildren() const {
  return m_members.size();
}

WidgetRef<Widget> Widget::getChildNum(size_t num) const {
  return WidgetRef<Widget>(*m_members.at(num));
}

String const& Widget::name() const {
  return m_name;
}

void Widget::setName(String const& name) {
  m_name = name;
}

String Widget::fullName() const {
  if (m_parent) {
    return m_parent->fullName() + "." + name();
  }

  return name();
}

String Widget::toStringImpl(int indentLevel) const {
  auto leader = String(" ") * indentLevel;
  String childrenString;
  for (auto& child : m_members) {
    childrenString.append(child->toStringImpl(indentLevel + 4));
  }
  String output = strf(R"OUTPUT({}{} : {
{}  address : %p,
{}  visible : {},
{}  position : {},
{}  size : {},
{}  children : {
{}
{}  }
{}}
)OUTPUT",
      leader,
      m_name,
      leader,
      static_cast<const void*>(this),
      leader,
      m_visible ? "true" : "false",
      leader,
      m_position,
      leader,
      m_size,
      leader,
      childrenString,
      leader,
      leader);

  return output;
}

bool Widget::setLabel(String const& name, String const& value) {
  if (containsChild(name)) {
    auto child = fetchChild(name);
    if (auto label = as<LabelWidget>(child)) {
      label->setText(value);
      return true;
    }
  }
  return false;
}

std::ostream& operator<<(std::ostream& os, Widget const& widget) {
  os << widget.toStringImpl(0);
  return os;
}

}
