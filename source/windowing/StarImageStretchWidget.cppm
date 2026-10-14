module;
import std;
import star.json;

#include "StarCasting.hpp"
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.color;
import star.font;
import star.variant;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
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

class ImageStretchWidget;
using ImageStretchWidgetPtr = std::shared_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstPtr = std::shared_ptr<ImageStretchWidget const>;
using ImageStretchWidgetWeakPtr = std::weak_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstWeakPtr = std::weak_ptr<ImageStretchWidget const>;
using ImageStretchWidgetUPtr = std::unique_ptr<ImageStretchWidget>;
using ImageStretchWidgetConstUPtr = std::unique_ptr<ImageStretchWidget const>;

class ImageStretchWidget : public Widget {
public:
  ImageStretchWidget(ImageStretchSet const& imageStretchSet, GuiDirection direction);
  void setImageStretchSet(String const& beginImage, String const& innerImage, String const& endImage);

  virtual ~ImageStretchWidget() {}

protected:
  virtual void renderImpl();

private:
  ImageStretchSet m_imageStretchSet;
  GuiDirection m_direction;
};

}

export module star.image_stretch_widget;

export namespace Star {
  using ::Star::ImageStretchWidget;
  using ::Star::ImageStretchWidgetPtr;
  using ::Star::ImageStretchWidgetConstPtr;
  using ::Star::ImageStretchWidgetWeakPtr;
  using ::Star::ImageStretchWidgetConstWeakPtr;
  using ::Star::ImageStretchWidgetUPtr;
  using ::Star::ImageStretchWidgetConstUPtr;
}
