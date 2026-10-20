import std;
import star.memory;
import star.json;
import star.json_extra;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.random;

import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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
import star.widget_parsing;
import star.gui_reader;
import star.list_widget;
import star.image_widget;

namespace Star {

ListWidget::ListWidget(GuiContext& context, Json const& schema) : Widget(context), m_schema(schema), m_reader(context) {
  m_selectedItem = NPos;
  m_columns = 1;
  setSchema(m_schema);
  updateSizeAndPosition();
}

ListWidget::ListWidget(GuiContext& context) : Widget(context), m_reader(context) {
  m_selectedItem = NPos;
  m_columns = 1;
  updateSizeAndPosition();
}

RectI ListWidget::relativeBoundRect() const {
  if (m_fillDown)
    return RectI::withSize(relativePosition() - Vec2I(0, size()[1]), size());
  else
    return RectI::withSize(relativePosition(), size());
}

void ListWidget::setCallback(WidgetCallbackFunc callback) {
  m_callback = std::move(callback);
}

bool ListWidget::sendEvent(InputEvent const& event) {
  if (!m_visible)
    return false;

  for (std::size_t i = m_members.size(); i != 0; --i) {
    auto child = m_members[i - 1];
    if (child->sendEvent(event)
        || (event.is<MouseButtonDownEvent>() && child->inMember(*context()->mousePosition(event))
              && event.get<MouseButtonDownEvent>().mouseButton == MouseButton::Left)) {
      setSelected(i - 1);
      return true;
    }
    setHovered(i - 1, event.is<MouseMoveEvent>() && child->inMember(*context()->mousePosition(event)));
  }

  return false;
}

void ListWidget::setSchema(Json const& schema) {
  clear();
  m_schema = schema;
  try {
    m_selectedBG = schema.getString("selectedBG", "");
    m_unselectedBG = schema.getString("unselectedBG", "");
    m_hoverBG = schema.getString("hoverBG", "");
    m_disabledBG = schema.getString("disabledBG", "");
    if (m_disabledBG.empty() && !m_unselectedBG.empty())
      m_disabledBG = m_unselectedBG + this->context()->root().assets()->json("/interface.config:disabledButton").toString();
    m_spacing = jsonToVec2I(schema.get("spacing"));
    m_memberSize = jsonToVec2I(schema.get("memberSize"));
  } catch (JsonException const& e) {
    throw GuiException(strf("Missing required value in map: {}", outputException(e, false)));
  }
  updateSizeAndPosition();
}

std::shared_ptr<Widget> ListWidget::addItem() {
  auto newItem = constructWidget();
  addChild(toString(this->context()->root().random().randu64()), newItem);
  updateSizeAndPosition();

  return newItem;
}

std::shared_ptr<Widget> ListWidget::addItem(std::size_t at) {
  auto newItem = constructWidget();
  addChildAt(toString(this->context()->root().random().randu64()), newItem, at);
  updateSizeAndPosition();

  if (m_selectedItem != NPos && at <= m_selectedItem)
    setSelected(m_selectedItem + 1);

  return newItem;
}

std::shared_ptr<Widget> ListWidget::addItem(std::shared_ptr<Widget> existingItem) {
  addChild(toString(this->context()->root().random().randu64()), existingItem);
  updateSizeAndPosition();

  return existingItem;
}

std::shared_ptr<Widget> ListWidget::constructWidget() {
  std::shared_ptr<Widget> newItem = std::make_shared<Widget>(*this->context());
  m_reader.construct(m_schema.get("listTemplate"), newItem.get());
  newItem->setSize(m_memberSize);
  m_doScissor ? newItem->enableScissoring() : newItem->disableScissoring();
  return newItem;
}

void ListWidget::updateSizeAndPosition() {
  int rows = m_members.size() % m_columns ? m_members.size() / m_columns + 1 : m_members.size() / m_columns;
  for (std::size_t i = 0; i < m_members.size(); i++) {
    Vec2I currentOffset;
    int col = i % m_columns;
    int row = rows - (i / m_columns) - 1;
    if (m_fillDown)
      row -= rows;
    currentOffset = Vec2I((m_memberSize[0] + m_spacing[0]) * col, (m_memberSize[1] + m_spacing[1]) * row);
    if (!m_fillDown)
      currentOffset[1] += m_spacing[1];
    m_members[i]->setPosition(currentOffset);
  }
  if (m_members.size()) {
    auto width = (m_memberSize[0] + m_spacing[0]) * m_columns;
    auto height = (m_memberSize[1] + m_spacing[1]) * rows;
    setSize(Vec2I(width, height));
  } else {
    setSize(Vec2I());
  }
}

void ListWidget::setEnabled(std::size_t pos, bool enabled) {
  if (pos != NPos && pos < listSize()) {
    if (enabled) {
      m_disabledItems.remove(pos);
      if (auto bgWidget = itemAt(pos)->fetchChild<ImageWidget>("background"))
        bgWidget->setImage(pos == m_selectedItem ? m_selectedBG : m_unselectedBG);
    } else {
      m_disabledItems.add(pos);
      if (m_selectedItem == pos)
        clearSelected();
      if (auto bgWidget = itemAt(pos)->fetchChild<ImageWidget>("background"))
        bgWidget->setImage(m_disabledBG);
    }
  }
}

void ListWidget::setHovered(std::size_t pos, bool hovered) {
  if (m_hoverBG == "")
    return;

  if (pos != m_selectedItem && pos < listSize() && !m_disabledItems.contains(pos)) {
    if (auto bgWidget = itemAt(pos)->fetchChild<ImageWidget>("background")) {
      if (hovered)
        bgWidget->setImage(m_hoverBG);
      else
        bgWidget->setImage(m_unselectedBG);
    }
  }
}

void ListWidget::setSelected(std::size_t pos) {
  if ((m_selectedItem != NPos) && (m_selectedItem < listSize())) {
    if (auto bgWidget = selectedWidget()->fetchChild<ImageWidget>("background"))
      bgWidget->setImage(m_unselectedBG);
  }

  if (!m_disabledItems.contains(pos) && m_selectedItem != pos) {
    m_selectedItem = pos;
    if (m_callback)
      m_callback(this);
  }

  if (m_selectedItem != NPos) {
    if (auto bgWidget = selectedWidget()->fetchChild<ImageWidget>("background"))
      bgWidget->setImage(m_selectedBG);
  }
}

void ListWidget::clearSelected() {
  setSelected(NPos);
}

void ListWidget::setSelectedWidget(std::shared_ptr<Widget> selected) {
  auto offset = itemPosition(selected);

  if (offset == NPos) {
    throw GuiException("Attempted to select item not in list.");
  }

  setSelected(offset);
}

void ListWidget::registerMemberCallback(String const& name, WidgetCallbackFunc const& callback) {
  m_reader.registerCallback(name, callback);
}

void ListWidget::setFillDown(bool fillDown) {
  m_fillDown = fillDown;
}

void ListWidget::setColumns(std::uint64_t columns) {
  m_columns = columns;
}

void ListWidget::removeItem(std::size_t at) {
  removeChildAt(at);
  if (m_selectedItem == at)
    setSelected(NPos);
  else if (m_selectedItem != NPos && m_selectedItem > at)
    setSelected(m_selectedItem - 1);

  updateSizeAndPosition();
}

void ListWidget::removeItem(std::shared_ptr<Widget> item) {
  auto offset = itemPosition(item);

  if (offset == NPos) {
    throw GuiException("Attempted to remove item not in list.");
  }

  removeItem(offset);
}

void ListWidget::clear() {
  setSelected(NPos);
  removeAllChildren();
  updateSizeAndPosition();
}

std::size_t ListWidget::selectedItem() const {
  return m_selectedItem;
}

std::size_t ListWidget::itemPosition(std::shared_ptr<Widget> item) const {
  std::size_t offset = NPos;
  for (std::size_t i = 0; i < m_members.size(); ++i) {
    if (m_members[i] == item) {
      offset = i;
      break;
    }
  }
  return offset;
}

std::shared_ptr<Widget> ListWidget::itemAt(std::size_t n) const {
  if (n < m_members.size()) {
    return m_members[n];
  } else {
    return {};
  }
}

std::shared_ptr<Widget> ListWidget::selectedWidget() const {
  return itemAt(m_selectedItem);
}

List<std::shared_ptr<Widget>> const& ListWidget::list() const {
  return m_members;
}

std::size_t ListWidget::listSize() const {
  return numChildren();
}

}
