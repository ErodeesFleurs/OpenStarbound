import star.json_extra;
import std;
import star.memory;
import star.json;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.variant;
import star.strong_typedef;
import star.rpc_promise;
import star.vector;
import star.rect;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.lua;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.string;
import star.text;
import star.poly;
import star.asset_path;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;

import star.lua_root;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
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
import star.widget_parsing;
import star.gui_reader;
import star.widget_lua_bindings;
import star.lua_game_converters;
import star.canvas_widget;
import star.label_widget;
import star.list_widget;
import star.button_group;
import star.button_widget;
import star.text_box_widget;
import star.progress_widget;
import star.image_widget;
import star.slider_bar;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.layout;
import star.flow_layout;
import star.image_stretch_widget;
import star.scroll_area;


import star.item_bag;

import star.matrix3;
namespace Star {

LuaMethods<std::shared_ptr<CanvasWidget>> LuaUserDataMethods<std::shared_ptr<CanvasWidget>>::make() {
  LuaMethods<std::shared_ptr<CanvasWidget>> methods;

  methods.registerMethodWithSignature<Vec2I, std::shared_ptr<CanvasWidget>>("size", std::mem_fn(&CanvasWidget::size));
  methods.registerMethodWithSignature<Vec2I, std::shared_ptr<CanvasWidget>>("mousePosition", std::mem_fn(&CanvasWidget::mousePosition));

  methods.registerMethodWithSignature<void, std::shared_ptr<CanvasWidget>>("clear", std::mem_fn(&CanvasWidget::clear));

  methods.registerMethod("drawDrawable", [](std::shared_ptr<CanvasWidget> canvasWidget, Drawable drawable, std::optional<Vec2F> screenPos) {
    canvasWidget->drawDrawable(std::move(drawable), screenPos.value_or(Vec2F()));
  });

  methods.registerMethod("drawDrawables", [](std::shared_ptr<CanvasWidget> canvasWidget, List<Drawable> drawables, std::optional<Vec2F> screenPos) {
    Vec2F pos = screenPos.value_or(Vec2F());
    for (auto& drawable : drawables)
      canvasWidget->drawDrawable(std::move(drawable), pos);
  });

  methods.registerMethod("drawJsonDrawable", [](std::shared_ptr<CanvasWidget> canvasWidget, Json drawable, std::optional<Vec2F> screenPos) {
    canvasWidget->drawDrawable(Drawable(*canvasWidget->context()->root().imageMetadataDatabase(), drawable), screenPos.value_or(Vec2F()));
  });

  methods.registerMethod("drawJsonDrawables", [](std::shared_ptr<CanvasWidget> canvasWidget, JsonArray drawables, std::optional<Vec2F> screenPos) {
    Vec2F pos = screenPos.value_or(Vec2F());
    for (auto& drawable : drawables)
      canvasWidget->drawDrawable(Drawable(*canvasWidget->context()->root().imageMetadataDatabase(), drawable), pos);
  });

  methods.registerMethod("drawImage",
      [](std::shared_ptr<CanvasWidget> canvasWidget, String image, Vec2F position, std::optional<float> scale, std::optional<Color> color, std::optional<bool> centered) {
        if (centered && *centered)
          canvasWidget->drawImageCentered(image, position, scale.value_or(1.0f), color.value_or(Color::White).toRgba());
        else
          canvasWidget->drawImage(image, position, scale.value_or(1.0f), color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawImageDrawable",
      [](std::shared_ptr<CanvasWidget> canvasWidget, String image, Vec2F position, MVariant<Vec2F, float> scale, std::optional<Color> color, std::optional<float> rotation) {
        auto drawable = Drawable::makeImage(*canvasWidget->context()->root().imageMetadataDatabase(), image, 1.0, true, {0.0, 0.0}, color.value_or(Color::White));
        if (auto s = scale.maybe<Vec2F>())
          drawable.transform(Mat3F::scaling(*s));
        else if(auto s = scale.maybe<float>())
          drawable.transform(Mat3F::scaling(*s));
        if (rotation)
          drawable.rotate(*rotation);
        canvasWidget->drawDrawable(drawable, position);
      });
  methods.registerMethod("drawImageRect",
      [](std::shared_ptr<CanvasWidget> canvasWidget, String image, RectF texCoords, RectF screenCoords, std::optional<Color> color) {
        canvasWidget->drawImageRect(image, texCoords, screenCoords, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawTiledImage",
      [](std::shared_ptr<CanvasWidget> canvasWidget, String image, Vec2D offset, RectF screenCoords, std::optional<float> scale, std::optional<Color> color) {
        canvasWidget->drawTiledImage(image, scale.value_or(1.0f), offset, screenCoords, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawLine",
      [](std::shared_ptr<CanvasWidget> canvasWidget, Vec2F begin, Vec2F end, std::optional<Color> color, std::optional<float> lineWidth) {
        canvasWidget->drawLine(begin, end, color.value_or(Color::White).toRgba(), lineWidth.value_or(1.0f));
      });
  methods.registerMethod("drawRect",
      [](std::shared_ptr<CanvasWidget> canvasWidget, RectF rect, std::optional<Color> color) {
        canvasWidget->drawRect(rect, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawPoly",
      [](std::shared_ptr<CanvasWidget> canvasWidget, PolyF poly, std::optional<Color> color, std::optional<float> lineWidth) {
        canvasWidget->drawPoly(poly, color.value_or(Color::White).toRgba(), lineWidth.value_or(1.0f));
      });
  methods.registerMethod("drawTriangles",
      [](std::shared_ptr<CanvasWidget> canvasWidget, List<PolyF> triangles, std::optional<Color> color) {
        auto tris = triangles.transformed([](PolyF const& poly) {
            if (poly.sides() != 3)
              throw StarException("Triangle must have exactly 3 sides");
            return std::tuple<Vec2F, Vec2F, Vec2F>(poly.vertex(0), poly.vertex(1), poly.vertex(2));
          });
        canvasWidget->drawTriangles(tris, color.value_or(Color::White).toRgba());
      });
  methods.registerMethod("drawText",
      [](std::shared_ptr<CanvasWidget> canvasWidget, String text, Json tp, unsigned fontSize, std::optional<Color> color, std::optional<float> lineSpacing, std::optional<String> font, std::optional<String> directives) {
        canvasWidget->drawText(text, TextPositioning(tp), fontSize, color.value_or(Color::White).toRgba(), FontMode::Normal, lineSpacing.value_or(DefaultLineSpacing), font.value_or(""), directives.value_or(""));
      });

  return methods;
}

namespace LuaBindings {

LuaCallbacks makeWidgetCallbacks(GuiContext& context, Widget* parentWidget, std::shared_ptr<GuiReader> reader) {
  if (!reader)
    reader = std::make_shared<GuiReader>(context);

  LuaCallbacks callbacks;

  // a bit miscellaneous, but put this here since widgets have access to gui context

  callbacks.registerCallback("playSound",
      [parentWidget](String const& audio, std::optional<int> loops, std::optional<float> volume) -> std::shared_ptr<AudioInstance> {
        return parentWidget->context()->playAudio(audio, loops.value_or(0), volume.value_or(1.0f));
      });

  // widget userdata methods

  callbacks.registerCallback("bindCanvas", [parentWidget](String const& widgetName) -> std::optional<std::shared_ptr<CanvasWidget>> {
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
      if (auto widget = parentWidget->fetchChild<Widget>(widgetName)) {
        String name = newChildName.value_or(toString(parentWidget->context()->root().random().randu64()));
        if (auto newChild = reader->makeSingle(name, newChildConfig))
          widget->addChild(name, newChild);
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
        return slider->setRange(newMin, newMax, newDelta.value_or(1));
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

  callbacks.registerCallback("removeListItem", [parentWidget](String const& widgetName, std::size_t at) {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        list->removeItem(at);
    });

  callbacks.registerCallback("getListSelected", [parentWidget](String const& widgetName) -> std::optional<String> {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        if (list->selectedItem() != NPos)
          return list->selectedWidget()->name();
      return {};
    });

  callbacks.registerCallback("setListSelected", [parentWidget](String const& widgetName, String const& selectedName) {
      if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
        if (auto selected = list->fetchChild(selectedName))
          list->setSelectedWidget(selected);
    });

  callbacks.registerCallback("clearListSelected", [parentWidget](String const& widgetName) {
    if (auto list = parentWidget->fetchChild<ListWidget>(widgetName))
      list->clearSelected();
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
        auto itemDb = parentWidget->context()->root().itemDatabase();
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
        std::shared_ptr<Widget> newChild = std::make_shared<ImageWidget>(*parentWidget->context(), image);
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

}
