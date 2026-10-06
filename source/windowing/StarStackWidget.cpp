#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarEither.hpp"
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
import star.stack_widget;

namespace Star {

void StackWidget::showPage(size_t page) {
  if (m_shownPage)
    m_shownPage->hide();
  m_shownPage = m_members[page];
  m_page = makeLeft(page);
  if (m_shownPage)
    m_shownPage->show();
}

void StackWidget::showPage(String const& name) {
  if (m_shownPage)
    m_shownPage->hide();
  m_shownPage = m_memberHash.get(name);
  m_page = makeRight(name);
  if (m_shownPage)
    m_shownPage->show();
}

Either<size_t, String> StackWidget::currentPage() const {
  return m_page;
}

void StackWidget::addChild(String const& name, WidgetPtr member) {
  Widget::addChild(name, member);
  if (m_members.size() != 1)
    member->hide();
  else
    showPage(0);
}

}
