module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
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

namespace Star {

STAR_CLASS(ImageWidget);
class ImageWidget : public Widget {
public:
  ImageWidget(String const& image = {});

  bool interactive() const override;
  void setImage(String const& image);
  void setScale(float scale);
  void setRotation(float rotation);
  String image() const;

  void setDrawables(List<Drawable> drawables);
  Vec2I offset();
  void setOffset(Vec2I const& offset);
  bool centered();
  void setCentered(bool centered);
  bool trim();
  void setTrim(bool trim);

  void setMaxSize(Vec2I const& size);
  void setMinSize(Vec2I const& size);

  RectI screenBoundRect() const override;

protected:
  void renderImpl() override;

private:
  void transformDrawables();

  List<Drawable> m_baseDrawables;
  List<Drawable> m_drawables;
  bool m_centered;
  bool m_trim;
  float m_scale;
  float m_rotation;
  Vec2I m_offset;
  Vec2I m_maxSize;
  Vec2I m_minSize;
};

}

export module star.image_widget;

export namespace Star {
  using ::Star::ImageWidget;
  using ::Star::ImageWidgetPtr;
  using ::Star::ImageWidgetConstPtr;
  using ::Star::ImageWidgetWeakPtr;
  using ::Star::ImageWidgetConstWeakPtr;
  using ::Star::ImageWidgetUPtr;
  using ::Star::ImageWidgetConstUPtr;
}
