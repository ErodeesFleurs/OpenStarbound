#include "StarTabSet.hpp"
#include "StarButtonWidget.hpp"
#include "StarStackWidget.hpp"
#include "StarFlowLayout.hpp"
#include "StarGuiReader.hpp"
#include "StarLexicalCast.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

TabSetWidget::TabSetWidget(GuiContext& context, TabSetConfig const& tabSetConfig) : Widget(context) {
  m_tabSetConfig = tabSetConfig;

  auto tabBar = make_unique<FlowLayout>(context);
  tabBar->setSpacing(m_tabSetConfig.tabButtonSpacing);
  m_tabBar = WidgetRef<FlowLayout>(*tabBar);
  Widget::addChild("tabBar", std::move(tabBar));

  auto stack = make_unique<StackWidget>(context);
  m_stack = WidgetRef<StackWidget>(*stack);
  addChild("tabs", std::move(stack));

  markAsContainer();
}

void TabSetWidget::setSize(Vec2I const& size) {
  auto& guiContext = context();
  auto const& imgMetadata = guiContext.imageMetadata();
  auto tabHeight = max({imgMetadata->imageSize(m_tabSetConfig.tabButtonBaseImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonHoverImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonPressedImage).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonBaseImageSelected).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonHoverImageSelected).y(),
      imgMetadata->imageSize(m_tabSetConfig.tabButtonPressedImageSelected).y()});

  Widget::setSize(Vec2I(size.x(), max<int>(size.y(), tabHeight)));

  m_tabBar->setSize({size.x(), tabHeight});
  m_tabBar->setPosition({0, size.y() - tabHeight});
  m_stack->setSize({size.x(), size.y() - tabHeight});
}

void TabSetWidget::addTab(String const& widgetName, UniquePtr<Widget> widget, String const& title) {
  auto newButton = make_unique<ButtonWidget>(context());
  newButton->setImages(
      m_tabSetConfig.tabButtonBaseImage, m_tabSetConfig.tabButtonHoverImage, m_tabSetConfig.tabButtonPressedImage);
  newButton->setCheckedImages(m_tabSetConfig.tabButtonBaseImageSelected,
      m_tabSetConfig.tabButtonHoverImageSelected,
      m_tabSetConfig.tabButtonPressedImageSelected);
  newButton->setCheckable(true);
  newButton->setText(title);
  newButton->setTextOffset(m_tabSetConfig.tabButtonTextOffset);
  newButton->setPressedOffset(m_tabSetConfig.tabButtonPressedOffset);

  size_t pageForButton = m_tabBar->numChildren();
  newButton->setCallback([this, pageForButton](Widget*) { tabSelect(pageForButton); });

  m_tabBar->addChild(toString(pageForButton), std::move(newButton));
  m_stack->addChild(widgetName, std::move(widget));

  if (!m_lastSelected)
    tabSelect(0);
}

size_t TabSetWidget::tabCount() const {
  return m_tabBar->numChildren();
}

void TabSetWidget::tabSelect(size_t page) {
  if (m_lastSelected != page) {
    m_lastSelected = page;
    m_stack->showPage(page);
    for (size_t i = 0; i < m_tabBar->numChildren(); ++i) {
      if (i == page)
        m_tabBar->getChildNum<ButtonWidget>(i)->setChecked(true);
      else
        m_tabBar->getChildNum<ButtonWidget>(i)->setChecked(false);
    }
    if (m_callback)
      m_callback(this);
  } else {
    m_tabBar->getChildNum<ButtonWidget>(page)->setChecked(true);
  }
}

size_t TabSetWidget::selectedTab() const {
  return m_lastSelected.value(NPos);
}

void TabSetWidget::setCallback(WidgetCallbackFunc callback) {
  m_callback = std::move(callback);
}

}
