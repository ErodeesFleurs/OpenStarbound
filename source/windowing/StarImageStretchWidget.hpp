#pragma once

#include "StarWidget.hpp"

namespace Star {

class ImageStretchWidget;
using ImageStretchWidgetPtr = SharedPtr<ImageStretchWidget>;

class ImageStretchWidget : public Widget {
public:
  ImageStretchWidget(ImageStretchSet const& imageStretchSet, GuiDirection direction);
  void setImageStretchSet(String const& beginImage, String const& innerImage, String const& endImage);

  virtual ~ImageStretchWidget() = default;

protected:
  virtual void renderImpl() override;

private:
  ImageStretchSet m_imageStretchSet;
  GuiDirection m_direction;
};

}
