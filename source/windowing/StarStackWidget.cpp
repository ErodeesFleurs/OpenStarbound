#include "StarStackWidget.hpp"

namespace Star {

StackWidget::StackWidget(GuiContext& context) : Widget(context) {}

void StackWidget::showPage(size_t page) {
  if (m_shownPage)
    m_shownPage->hide();
  m_shownPage = WidgetRef<Widget>(*m_members[page]);
  m_page = makeLeft(page);
  if (m_shownPage)
    m_shownPage->show();
}

void StackWidget::showPage(String const& name) {
  if (m_shownPage)
    m_shownPage->hide();
  if (auto index = m_memberHash.maybe(name))
    m_shownPage = WidgetRef<Widget>(*m_members[*index]);
  else
    m_shownPage = nullptr;
  m_page = makeRight(name);
  if (m_shownPage)
    m_shownPage->show();
}

Either<size_t, String> StackWidget::currentPage() const {
  return m_page;
}

void StackWidget::addChild(String const& name, UniquePtr<Widget> member) {
  Widget::addChild(name, std::move(member));
  if (m_members.size() != 1)
    m_members.back()->hide();
  else
    showPage(0);
}

}
