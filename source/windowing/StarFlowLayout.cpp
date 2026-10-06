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
import star.layout;
import star.flow_layout;

namespace Star {

FlowLayout::FlowLayout() : m_wrap(true) {}

void FlowLayout::update(float dt) {
  Layout::update(dt);

  int consumedWidth = 0;
  int rowHeight = 0;
  Vec2I currentOffset = {0, size()[1]};
  for (auto child : m_members) {
    if (m_wrap && consumedWidth + child->size()[0] > size()[0] && consumedWidth != 0) { // wrapping
      currentOffset[0] = 0;
      consumedWidth = 0;
      currentOffset[1] -= rowHeight + m_spacing[1];
    }
    if (rowHeight < child->size()[1]) {
      rowHeight = child->size()[1];
    }
    child->setPosition(Vec2I{currentOffset[0], currentOffset[1] - child->size()[1]});
    consumedWidth += child->size()[0] + m_spacing[0];
    currentOffset[0] = consumedWidth;
  }
}

void FlowLayout::setSpacing(Vec2I const& spacing) {
  m_spacing = spacing;
}

void FlowLayout::setWrapping(bool wrap) {
  m_wrap = wrap;
}

}
