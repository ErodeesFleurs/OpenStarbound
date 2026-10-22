module;
import std;
import star.type_identity;
import star.format;
import star.json;

import star.casting;
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
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
import star.pane;

namespace Star {

class PopupInterface : public Pane {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<PopupInterface, Pane>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<PopupInterface>();
  }

  [[nodiscard]] static std::expected<std::shared_ptr<PopupInterface>, StarException> create(GuiContext& guiContext);

  virtual ~PopupInterface() {}

  [[nodiscard]] std::expected<void, StarException> displayMessage(String const& message, String const& title, String const& subtitle, std::optional<String> const& onShowSound = {});

private:
  explicit PopupInterface(GuiContext& guiContext);
};

}

export module star.popup_interface;

export namespace Star {
  using ::Star::PopupInterface;
}
