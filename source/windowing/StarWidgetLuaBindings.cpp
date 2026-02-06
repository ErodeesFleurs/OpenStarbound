#include "StarWidgetLuaBindings.hpp"
#include "StarJsonExtra.hpp" // IWYU pragma: keep
#include "StarLuaGameConverters.hpp" // IWYU pragma: keep
#include "StarGuiReader.hpp"
#include "StarCanvasWidget.hpp"
#include "StarLabelWidget.hpp"
#include "StarListWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarButtonGroup.hpp"
#include "StarTextBoxWidget.hpp"
#include "StarProgressWidget.hpp"
#include "StarSliderBar.hpp"
#include "StarItemGridWidget.hpp"
#include "StarItemSlotWidget.hpp"
#include "StarItemDatabase.hpp" // IWYU pragma: keep
#include "StarFlowLayout.hpp"
#include "StarImageStretchWidget.hpp"
#include "StarScrollArea.hpp"

#include <optional>

namespace Star {

LuaMethods<CanvasWidgetPtr> LuaUserDataMethods<CanvasWidgetPtr>::make() {
  LuaMethods<CanvasWidgetPtr> methods;

  methods.registerMethodWithSignature<Vec2I, CanvasWidgetPtr>("size", mem_fn(&CanvasWidget::size));
  methods.registerMethodWithSignature<Vec2I, CanvasWidgetPtr>("mousePosition", mem_fn(&CanvasWidget::mousePosition));

  methods.registerMethodWithSignature<void, CanvasWidgetPtr>("clear", mem_fn(&CanvasWidget::clear));

  methods.registerMethod("drawDrawable", [](CanvasWidgetPtr canvasWidget, Drawable drawable, std::optional<Vec2F> screenPos) {
    canvasWidget->drawDrawable(std::move(drawable), screenPos.value_or(Vec2F()));
  });

  methods.registerMethod("drawDrawables", [](CanvasWidgetPtr canvasWidget, List<Drawable> drawables, std::optional<Vec2F> screenPos) {
    Vec2F pos = screenPos.value_or(Vec2F());
    for (auto& drawable : drawables)
      canvasWidget->drawDrawable(std::move(drawable), pos);
  });

  methods.registerMethod("drawJsonDrawable", [](CanvasWidgetPtr canvasWidget, Json drawable, std::optional<Vec2F> screenPos) {
    canvasWidget->drawDrawable(Drawable(drawable), screenPos.value_or(Vec2F()));
  });

  methods.registerMethod("drawJsonDrawables", [](CanvasWidgetPtr canvasWidget, JsonArray drawables, std::optional<Vec2F> screenPos) {
    Vec2F pos = screenPos.value_or(Vec2F());
    for (auto& drawable : drawables)
      canvasWidget->drawDrawable(Drawable(drawable), pos);
  });

  methods.registerMethod("drawImage",
      [](CanvasWidgetPtr canvasWidget, String image, Vec2F position, std::optional<float> scale, std::optional<Color> color, std::optional<bool> centered) {
        if (centered && *centered)
          canvasWidget->drawImageCentered(image, position, scale.value_or(1.0f), color.value_or(Color::White).toRgba());
        else
          canvasWidget->drawImage(image, position, scale.value_or(1.0f), color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawImageDrawable",
      [](CanvasWidgetPtr canvasWidget, String image, Vec2F position, MVariant<Vec2F, float> scale, std::optional<Color> color, std::optional<float> rotation) {
        auto drawable = Drawable::makeImage(image, 1.0, true, {0.0, 0.0}, color.value_or(Color::White));
        if (auto s = scale.maybe<Vec2F>())
          drawable.transform(Mat3F::scaling(*s));
        else if(auto s = scale.maybe<float>())
          drawable.transform(Mat3F::scaling(*s));
        if (rotation)
          drawable.rotate(*rotation);
        canvasWidget->drawDrawable(drawable, position);
      });
  methods.registerMethod("drawImageRect",
      [](CanvasWidgetPtr canvasWidget, String image, RectF texCoords, RectF screenCoords, std::optional<Color> color) {
        canvasWidget->drawImageRect(image, texCoords, screenCoords, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawTiledImage",
      [](CanvasWidgetPtr canvasWidget, String image, Vec2D offset, RectF screenCoords, std::optional<float> scale, std::optional<Color> color) {
        canvasWidget->drawTiledImage(image, scale.value_or(1.0f), offset, screenCoords, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawLine",
      [](CanvasWidgetPtr canvasWidget, Vec2F begin, Vec2F end, std::optional<Color> color, std::optional<float> lineWidth) {
        canvasWidget->drawLine(begin, end, color.value_or(Color::White).toRgba(), lineWidth.value_or(1.0f));
      });
  methods.registerMethod("drawRect",
      [](CanvasWidgetPtr canvasWidget, RectF rect, std::optional<Color> color) {
        canvasWidget->drawRect(rect, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawPoly",
      [](CanvasWidgetPtr canvasWidget, PolyF poly, std::optional<Color> color, std::optional<float> lineWidth) {
        canvasWidget->drawPoly(poly, color.value_or(Color::White).toRgba(), lineWidth.value_or(1.0f));
      });
  methods.registerMethod("drawTriangles",
      [](CanvasWidgetPtr canvasWidget, List<PolyF> triangles, std::optional<Color> color) {
        auto tris = triangles.transformed([](PolyF const& poly) {
            if (poly.sides() != 3)
              throw StarException("Triangle must have exactly 3 sides");
            return tuple<Vec2F, Vec2F, Vec2F>(poly.vertex(0), poly.vertex(1), poly.vertex(2));
          });
        canvasWidget->drawTriangles(tris, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawText",
      [](CanvasWidgetPtr canvasWidget, String text, Json tp, unsigned fontSize, std::optional<Color> color, std::optional<float> lineSpacing, std::optional<String> font, std::optional<String> directives) {
        canvasWidget->drawText(text, TextPositioning(tp), fontSize, color.value_or(Color::White).toRgba(), FontMode::Normal, lineSpacing.value_or(DefaultLineSpacing), font.value_or(""), directives.value_or(""));
      });

  return methods;
}

LuaCallbacks LuaBindings::makeWidgetCallbacks(Widget* parentWidget, GuiReaderPtr reader) {
  if (!reader)
    reader = make_shared<GuiReader>();

  LuaCallbacks callbacks;

  // a bit miscellaneous, but put this here since widgets have access to gui context

  callbacks.registerCallback("playSound",
      [parentWidget](String const& audio, std::optional<int> loops, std::optional<float> volume) {
        parentWidget->context()->playAudio(audio, loops.value_or(0), volume.value_or(1.0f));
      });

  // widget userdata methods

  callbacks.registerCallback("bindCanvas", [parentWidget](String const& widgetName) -> std::optional<CanvasWidgetPtr> {
      if (auto canvas = parentWidget->fetchChild<CanvasWidget>(widgetName))
        return canvas;
      return {};
    });

  // generic widget callbacks

  callbacks.registerCallback("getPosition", [parentWidget](String const& widgetName) -> std::optional<Vec2I> {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        return widget->relativePosition();
      return {};
    });
  callbacks.registerCallback("setPosition", [parentWidget](String const& widgetName, Vec2I const& position) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->setPosition(position);
    });

  callbacks.registerCallback("getSize", [parentWidget](String const& widgetName) -> std::optional<Vec2I> {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        return widget->size();
      return {};
    });
  callbacks.registerCallback("setSize", [parentWidget](String const& widgetName, Vec2I const& size) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->setSize(size);
    });

  callbacks.registerCallback("setVisible", [parentWidget](String const& widgetName, bool visible) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->setVisibility(visible);
    });

  callbacks.registerCallback("active", [parentWidget](String const& widgetName) -> std::optional<bool> {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        return widget->active();
      return {};
    });

  callbacks.registerCallback("focus", [parentWidget](String const& widgetName) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->focus();
    });

  callbacks.registerCallback("hasFocus", [parentWidget](String const& widgetName) -> std::optional<bool> {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        return widget->hasFocus();
      return {};
    });

  callbacks.registerCallback("blur", [parentWidget](String const& widgetName) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->blur();
    });

  callbacks.registerCallback("getData", [parentWidget](String const& widgetName) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        return widget->data();
      return Json();
    });

  callbacks.registerCallback("setData", [parentWidget](String const& widgetName, Json const& data) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->setData(data);
    });

  callbacks.registerCallback("getChildAt", [parentWidget](Vec2I const& screenPosition) -> std::optional<String> {
      if (auto widget = parentWidget->getChildAt(screenPosition))
        return widget->fullName();
      else
        return{};
    });

  callbacks.registerCallback("inMember", [parentWidget](String const& widgetName, Vec2I const& screenPosition) -> std::optional<bool> {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        return widget->inMember(screenPosition);
      else
        return {};
    });

  callbacks.registerCallback("addChild", [parentWidget, reader](String const& widgetName, Json const& newChildConfig, std::optional<String> const& newChildName) {
      if (auto parent = parentWidget->fetchChild(widgetName)) {
        String name = newChildName.value_or(toString(Random::randu64()));
        WidgetPtr newChild = reader->makeSingle(name, newChildConfig);
        parent->addChild(name, newChild);
      }
    });

  callbacks.registerCallback("removeAllChildren", [parentWidget](String const& widgetName) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->removeAllChildren();
    });

  callbacks.registerCallback("removeChild", [parentWidget](String const& widgetName, String const& childName) {
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName))
        widget->removeChild(childName);
    });

  // callbacks only valid for specific widget types

  callbacks.registerCallback("setHint", [parentWidget](String const& widgetName, String const& hint) {
    if (auto widget = parentWidget->fetchChild(widgetName)) {
      if (auto textBox = as<TextBoxWidget>(widget))
        textBox->setHint(hint);
    }
  });

  callbacks.registerCallback("getHint", [parentWidget](String const& widgetName) -> std::optional<String> {
    if (auto widget = parentWidget->fetchChild(widgetName)) {
      if (auto textBox = as<TextBoxWidget>(widget))
        return textBox->getHint();
    }
    return {};
  });

  callbacks.registerCallback("setCursorPosition", [parentWidget](String const& widgetName, int cursorPosition) {
    if (auto widget = parentWidget->fetchChild(widgetName)) {
      if (auto textBox = as<TextBoxWidget>(widget))
        textBox->setCursorPosition(cursorPosition);
    }
  });

  callbacks.registerCallback("getCursorPosition", [parentWidget](String const& widgetName) -> std::optional<int> {
    if (auto widget = parentWidget->fetchChild(widgetName)) {
      if (auto textBox = as<TextBoxWidget>(widget))
        return textBox->getCursorPosition();
    }
    return {};
  });

  callbacks.registerCallback("getText", [parentWidget](String const& widgetName) -> std::optional<String> {
      if (auto widget = parentWidget->fetchChild(widgetName)) {
        if (auto label = as<LabelWidget>(widget))
          return label->text();
        else if (auto button = as<ButtonWidget>(widget))
          return button->getText();
        else if (auto textBox = as<TextBoxWidget>(widget))
          return textBox->getText();
      }
      return {};
    });

  callbacks.registerCallback("setText", [parentWidget](String const& widgetName, String const& text) {
      if (auto widget = parentWidget->fetchChild(widgetName)) {
        if (auto label = as<LabelWidget>(widget))
          label->setText(text);
        else if (auto button = as<ButtonWidget>(widget))
          button->setText(text);
        else if (auto textBox = as<TextBoxWidget>(widget))
          textBox->setText(text);
      }
    });

  callbacks.registerCallback("setFontColor", [parentWidget](String const& widgetName, Color const& color) {
      if (auto widget = parentWidget->fetchChild(widgetName)) {
        if (auto label = as<LabelWidget>(widget))
          label->setColor(color);
        else if (auto button = as<ButtonWidget>(widget))
          button->setFontColor(color);
        else if (auto textBox = as<TextBoxWidget>(widget))
          textBox->setColor(color);
      }
    });

  callbacks.registerCallback("setImage", [parentWidget](String const& widgetName, String const& imagePath) {
      if (auto image = parentWidget->fetchChild<ImageWidget>(widgetName))
        image->setImage(imagePath);
    });

  callbacks.registerCallback("setImageScale", [parentWidget](String const& widgetName, float const& imageScale) {
      if (auto image = parentWidget->fetchChild<ImageWidget>(widgetName))
        image->setScale(imageScale);
    });

  callbacks.registerCallback("setImageRotation", [parentWidget](String const& widgetName, float const& imageRotation) {
      if (auto image = parentWidget->fetchChild<ImageWidget>(widgetName))
        image->setRotation(imageRotation);
    });

  callbacks.registerCallback("setButtonEnabled", [parentWidget](String const& widgetName, bool enabled) {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        button->setEnabled(enabled);
    });

  callbacks.registerCallback("setButtonImage", [parentWidget](String const& widgetName, String const& baseImage) {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        button->setImages(baseImage);
    });

  callbacks.registerCallback("setButtonImages", [parentWidget](String const& widgetName, Json const& imageSet) {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        button->setImages(imageSet.getString("base"), imageSet.getString("hover", ""), imageSet.getString("pressed", ""), imageSet.getString("disabled", ""));
    });

  callbacks.registerCallback("setButtonCheckedImages", [parentWidget](String const& widgetName, Json const& imageSet) {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        button->setCheckedImages(imageSet.getString("base"), imageSet.getString("hover", ""), imageSet.getString("pressed", ""), imageSet.getString("disabled", ""));
    });

  callbacks.registerCallback("setButtonOverlayImage", [parentWidget](String const& widgetName, String const& overlayImage) {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        button->setOverlayImage(overlayImage);
    });

  callbacks.registerCallback("getChecked", [parentWidget](String const& widgetName) -> std::optional<bool> {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        return button->isChecked();
      return {};
    });

  callbacks.registerCallback("setChecked", [parentWidget](String const& widgetName, bool checked) {
      if (auto button = parentWidget->fetchChild<ButtonWidget>(widgetName))
        button->setChecked(checked);
    });

  callbacks.registerCallback("getSelectedOption", [parentWidget](String const& widgetName) -> std::optional<int> {
      if (auto buttonGroup = parentWidget->fetchChild<ButtonGroupWidget>(widgetName))
        return buttonGroup->checkedId();
      return {};
    });

  callbacks.registerCallback("getSelectedData", [parentWidget](String const& widgetName) -> Json {
      if (auto buttonGroup = parentWidget->fetchChild<ButtonGroupWidget>(widgetName)) {
        if (auto button = buttonGroup->checkedButton())
          return button->data();
      }
      return {};
    });

  callbacks.registerCallback("setSelectedOption", [parentWidget](String const& widgetName, std::optional<int> index) {
      if (auto buttonGroup = parentWidget->fetchChild<ButtonGroupWidget>(widgetName))
        buttonGroup->select(index.value_or(ButtonGroup::NoButton));
    });

  callbacks.registerCallback("setOptionEnabled", [parentWidget](String const& widgetName, int index, bool enabled) {
      if (auto buttonGroup = parentWidget->fetchChild<ButtonGroupWidget>(widgetName)) {
        if (auto button = buttonGroup->button(index))
          button->setEnabled(enabled);
      }
    });

  callbacks.registerCallback("setOptionVisible", [parentWidget](String const& widgetName, int index, bool visible) {
      if (auto buttonGroup = parentWidget->fetchChild<ButtonGroupWidget>(widgetName)) {
        if (auto button = buttonGroup->button(index))
          button->setVisibility(visible);
      }
    });

  callbacks.registerCallback("setProgress", [parentWidget](String const& widgetName, float const& value) {
      if (auto progress = parentWidget->fetchChild<ProgressWidget>(widgetName))
        progress->setCurrentProgressLevel(value);
    });

  callbacks.registerCallback("setSliderEnabled", [parentWidget](String const& widgetName, bool enabled) {
      if (auto slider = parentWidget->fetchChild<SliderBarWidget>(widgetName))
        slider->setEnabled(enabled);
    });

  callbacks.registerCallback("getSliderValue", [parentWidget](String const& widgetName) -> std::optional<int> {
      if (auto slider = parentWidget->fetchChild<SliderBarWidget>(widgetName))
        return slider->val();
      return {};
    });

  callbacks.registerCallback("setSliderValue", [parentWidget](String const& widgetName, int newValue) {
      if (auto slider = parentWidget->fetchChild<SliderBarWidget>(widgetName))
        return slider->setVal(newValue);
    });

  callbacks.registerCallback("setSliderRange", [parentWidget](String const& widgetName, int newMin, int newMax, std::optional<int> newDelta) {
      if (auto slider = parentWidget->fetchChild<SliderBarWidget>(widgetName))
        slider->setRange(newMin, newMax, newDelta.value_or(1));
    });

  callbacks.registerCallback("clearListItems", [parentWidget](String const& widgetName) {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        list->clear();
    });

  callbacks.registerCallback("addListItem", [parentWidget](String const& widgetName) -> std::optional<String> {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName)) {
        auto newItem = list->addItem();
        return newItem->name();
      }
      return {};
    });

  callbacks.registerCallback("removeListItem", [parentWidget](String const& widgetName, size_t at) {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        list->removeItem(at);
    });

  callbacks.registerCallback("getListSelected", [parentWidget](String const& widgetName) -> std::optional<String> {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        if (list->selectedItem() != std::numeric_limits<std::size_t>::max())
          return list->selectedWidget()->name();
      return {};
    });

  callbacks.registerCallback("setListSelected", [parentWidget](String const& widgetName, String const& selectedName) {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        if (auto selected = list->fetchChild(selectedName))
          list->setSelectedWidget(selected);
    });

  callbacks.registerCallback("registerMemberCallback", [parentWidget](String const& widgetName, String const& name, LuaFunction callback) {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName)){
        list->registerMemberCallback(name, [callback](Widget* widget) {
            callback.invoke(widget->name(), widget->data());
          });
      }
    });

  callbacks.registerCallback("itemGridItems", [parentWidget](String const& widgetName) {
      if (auto itemGrid = parentWidget->fetchChild<ItemGridWidget>(widgetName))
        return itemGrid->bag()->toJson();
      return Json();
    });

  callbacks.registerCallback("itemSlotItem", [parentWidget](String const& widgetName) -> std::optional<Json> {
      if (auto itemSlot = parentWidget->fetchChild<ItemSlotWidget>(widgetName)) {
        if (itemSlot->item())
          return itemSlot->item()->descriptor().toJson();
      }
      return {};
    });

  callbacks.registerCallback("setItemSlotItem", [parentWidget](String const& widgetName, Json const& item) {
      if (auto itemSlot = parentWidget->fetchChild<ItemSlotWidget>(widgetName)) {
        auto itemDb = Root::singleton().itemDatabase();
        itemSlot->setItem(itemDb->fromJson(item));
      }
    });

  callbacks.registerCallback("setItemSlotProgress", [parentWidget](String const& widgetName, float progress) {
      if (auto itemSlot = parentWidget->fetchChild<ItemSlotWidget>(widgetName)) {
        itemSlot->setProgress(progress);
      }
    });

  callbacks.registerCallback("addFlowImage", [parentWidget](String const& widgetName, String const& childName, String const& image) {
      if (auto flow = parentWidget->fetchChild<FlowLayout>(widgetName)) {
        WidgetPtr newChild = make_shared<ImageWidget>(image);
        flow->addChild(childName, newChild);
      }
    });

  callbacks.registerCallback("setImageStretchSet", [parentWidget](String const& widgetName, Json const& imageSet) {
      if (auto imageStretch = parentWidget->fetchChild<ImageStretchWidget>(widgetName)) {
        imageStretch->setImageStretchSet(imageSet.getString("begin", ""), imageSet.getString("inner", ""), imageSet.getString("end", ""));
      }
    });

  callbacks.registerCallback("getScrollOffset", [parentWidget](String const& widgetName) -> std::optional<Vec2I> {
    if (auto scrollArea = parentWidget->fetchChild<ScrollArea>(widgetName))
        return scrollArea->scrollOffset();
      return {};
  });

  callbacks.registerCallback("setScrollOffset", [parentWidget](String const& widgetName, Vec2I const& offset) {
    if (auto scrollArea = parentWidget->fetchChild<ScrollArea>(widgetName))
        scrollArea->scrollAreaBy(offset - scrollArea->scrollOffset());
  });

  callbacks.registerCallback("getMaxScrollPosition", [parentWidget](String const& widgetName) -> std::optional<Vec2I> {
    if (auto scrollArea = parentWidget->fetchChild<ScrollArea>(widgetName))
        return scrollArea->maxScrollPosition();
      return {};
  });

  return callbacks;
}

}
