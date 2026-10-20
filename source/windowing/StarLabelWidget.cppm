module;
import std;
import star.type_identity;
import star.json;

import star.casting;
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.rect;
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

class LabelWidget : public Widget {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<LabelWidget, Widget>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<LabelWidget>();
  }

  LabelWidget(GuiContext& context);
  LabelWidget(GuiContext& context, String text,
      Color const& color = Color::White,
      HorizontalAnchor const& hAnchor = HorizontalAnchor::LeftAnchor,
      VerticalAnchor const& vAnchor = VerticalAnchor::BottomAnchor,
      Maybe<unsigned> wrapWidth = {},
      Maybe<float> lineSpacing = {});

  String const& text() const;
  Maybe<unsigned> getTextCharLimit() const;
  void setText(String newText);
  void setFontSize(int fontSize);
  void setFontMode(FontMode fontMode);
  void setColor(Color newColor);
  void setAnchor(HorizontalAnchor hAnchor, VerticalAnchor vAnchor);
  void setWrapWidth(Maybe<unsigned> wrapWidth);
  void setLineSpacing(Maybe<float> lineSpacing);
  void setDirectives(String const& directives);
  void setTextCharLimit(Maybe<unsigned> charLimit);
  void setTextStyle(TextStyle const& style);
  void setFont(String const& font);

  RectI relativeBoundRect() const override;

protected:
  virtual RectI getScissorRect() const override;
  virtual void renderImpl() override;

private:
  void updateTextRegion();

  String m_text;
  TextStyle m_style;
  HorizontalAnchor m_hAnchor;
  VerticalAnchor m_vAnchor;
  Maybe<unsigned> m_wrapWidth;
  Maybe<float> m_lineSpacing;
  Maybe<unsigned> m_textCharLimit;
  RectI m_textRegion;
};

}

export module star.label_widget;

export namespace Star {
  using ::Star::LabelWidget;
}
