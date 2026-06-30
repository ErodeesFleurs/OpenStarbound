#pragma once

#include "StarWidget.hpp"
#include "StarGuiReader.hpp"

namespace Star {

class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;

class ListWidget : public Widget {
public:
  ListWidget(GuiContext& context, Json const& schema);
  explicit ListWidget(GuiContext& context);

  [[nodiscard]] RectI relativeBoundRect() const override;

  // Callback is called when the selection changes
  void setCallback(WidgetCallbackFunc callback);

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;
  void setSchema(Json const& schema);
  [[nodiscard]] UniquePtr<Widget> constructWidget();
  [[nodiscard]] WidgetRef<Widget> addItem();
  [[nodiscard]] WidgetRef<Widget> addItem(size_t at);
  void removeItem(size_t at);
  void removeItem(Widget& item);
  void clear();
  [[nodiscard]] size_t selectedItem() const;
  [[nodiscard]] size_t itemPosition(Widget& item) const;
  [[nodiscard]] WidgetRef<Widget> itemAt(size_t n) const;
  [[nodiscard]] WidgetRef<Widget> selectedWidget() const;
  [[nodiscard]] size_t listSize() const;
  void setEnabled(size_t pos, bool enabled);
  void setHovered(size_t pos, bool hovered);
  void setSelected(size_t pos);
  void clearSelected();
  void setSelectedWidget(WidgetRef<Widget> selected);

  void registerMemberCallback(String const& name, WidgetCallbackFunc const& callback);

  void setFillDown(bool fillDown);
  void setColumns(uint64_t columns);

private:
  void updateSizeAndPosition();

  Json m_schema;
  GuiReaderPtr m_reader;

  Set<size_t> m_disabledItems;
  size_t m_selectedItem = NPos;
  WidgetCallbackFunc m_callback;

  String m_selectedBG;
  String m_unselectedBG;
  String m_hoverBG;
  String m_disabledBG;
  Vec2I m_spacing;

  bool m_fillDown = false;
  uint64_t m_columns = 1;
};

}
