#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"

import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
import star.application_controller;
#include "StarVariant.hpp"
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
