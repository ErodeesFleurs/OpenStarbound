#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"

#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
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
import star.image_stretch_widget;

namespace Star {

ImageStretchWidget::ImageStretchWidget(ImageStretchSet const& imageStretchSet, GuiDirection direction)
  : m_imageStretchSet(imageStretchSet), m_direction(direction) {

}

void ImageStretchWidget::setImageStretchSet(String const& beginImage, String const& innerImage, String const& endImage) {
  m_imageStretchSet.begin = beginImage;
  m_imageStretchSet.inner = innerImage;
  m_imageStretchSet.end = endImage;
}

void ImageStretchWidget::renderImpl() {
  context()->drawImageStretchSet(m_imageStretchSet, RectF(screenBoundRect()), m_direction);
}

}
