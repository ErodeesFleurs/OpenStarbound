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
import star.vertical_layout;

namespace Star {

VerticalLayout::VerticalLayout(VerticalAnchor verticalAnchor, int verticalSpacing) {
  setVerticalAnchor(verticalAnchor);
  setVerticalSpacing(verticalSpacing);

  disableScissoring();
}

void VerticalLayout::update(float) {
  m_size = Vec2I(0, 0);

  if (m_members.empty())
    return;

  for (auto const& child : m_members) {
    auto childSize = child->size();
    m_size[1] += childSize[1];
    m_size[0] = max(m_size[0], childSize[0]);
  }
  m_size[1] += (m_members.size() - 1) * m_verticalSpacing;

  auto bounds = contentBoundRect();

  int verticalPos = m_fillDown ? bounds.yMax() : bounds.yMin();
  for (auto const& child : reverseIterate(m_members)) {
    auto childSize = child->size();

    Vec2I targetPosition;

    if (m_horizontalAnchor == HorizontalAnchor::LeftAnchor)
      targetPosition[0] = bounds.xMin();
    else if (m_horizontalAnchor == HorizontalAnchor::RightAnchor)
      targetPosition[0] = bounds.xMax() - childSize[0];
    else if (m_horizontalAnchor == HorizontalAnchor::HMidAnchor)
      targetPosition[0] = -childSize[0] / 2;

    if (m_fillDown) {
      verticalPos -= childSize[1];
      targetPosition[1] = verticalPos;
      verticalPos -= m_verticalSpacing;
    } else {
      targetPosition[1] = verticalPos;
      verticalPos -= childSize[1];
      verticalPos -= m_verticalSpacing;
    }

    // needed because position is included in relativeBoundRect
    child->setPosition(Vec2I(0, 0));

    child->setPosition(targetPosition - child->relativeBoundRect().min());
  }
}

Vec2I VerticalLayout::size() const {
  return m_size;
}

RectI VerticalLayout::relativeBoundRect() const {
  return contentBoundRect().translated(relativePosition());
}

void VerticalLayout::setHorizontalAnchor(HorizontalAnchor horizontalAnchor) {
  m_horizontalAnchor = horizontalAnchor;
  update(0);
}

void VerticalLayout::setVerticalAnchor(VerticalAnchor verticalAnchor) {
  m_verticalAnchor = verticalAnchor;
  update(0);
}

void VerticalLayout::setVerticalSpacing(int verticalSpacing) {
  m_verticalSpacing = verticalSpacing;
  update(0);
}

void VerticalLayout::setFillDown(bool fillDown) {
  m_fillDown = fillDown;
  update(0);
}

RectI VerticalLayout::contentBoundRect() const {
  auto min = Vec2I(0, 0);

  if (m_horizontalAnchor == HorizontalAnchor::RightAnchor)
    min[0] -= m_size[0];
  else if (m_horizontalAnchor == HorizontalAnchor::HMidAnchor)
    min[0] -= m_size[0] / 2;

  if (m_verticalAnchor == VerticalAnchor::TopAnchor)
    min[1] -= m_size[1];
  else if (m_verticalAnchor == VerticalAnchor::VMidAnchor)
    min[1] -= m_size[1] / 2;

  return RectI::withSize(min, m_size);
}

}
