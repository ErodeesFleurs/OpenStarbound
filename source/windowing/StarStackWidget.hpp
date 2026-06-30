#pragma once

#include "StarWidget.hpp"
#include "StarEither.hpp"

namespace Star {

class StackWidget;
using StackWidgetPtr = SharedPtr<StackWidget>;
class StackWidget : public Widget {
public:
  explicit StackWidget(GuiContext& context);

  void showPage(size_t page);
  void showPage(String const& name);

  [[nodiscard]] Either<size_t, String> currentPage() const;

  void addChild(String const& name, UniquePtr<Widget> member) override;

private:
  WidgetRef<Widget> m_shownPage;
  Either<size_t, String> m_page;
};

}
