module;
import star.json;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
#include "StarConfig.hpp"
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.variant;
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
import star.pane;

export module star.http_trust_dialog;

export namespace Star {

STAR_CLASS(HttpTrustDialog);

enum class HttpTrustReply {
  Allow,
  Deny
};

class HttpTrustDialog final : public Pane {
public:
  HttpTrustDialog();

  ~HttpTrustDialog() override;

  void displayRequest(String const& domain, function<void(HttpTrustReply, bool)> callback);

  void dismissed() override;

private:
  void reply(HttpTrustReply replyType);

  String m_domain;
  bool m_confirmed;
  function<void(HttpTrustReply, bool)> m_callback;
};

}
