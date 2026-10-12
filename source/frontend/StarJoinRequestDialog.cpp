#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.font;
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
import star.ref_ptr;
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
import star.game_types;
#include "StarSet.hpp"
import star.audio;
#include "StarList.hpp"
#include "StarMap.hpp"
import star.rpc_promise;
import star.random;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
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
import star.pane;


import star.join_request_dialog;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;
import star.button_group;
import star.button_widget;
import star.image_widget;

namespace Star {

JoinRequestDialog::JoinRequestDialog() : m_confirmed(false) {}

void JoinRequestDialog::displayRequest(String const& userName, function<void(P2PJoinRequestReply)> callback) {
  auto assets = Root::singleton().assets();

  removeAllChildren();

  GuiReader reader;

  m_callback = std::move(callback);

  reader.registerCallback("yes", [this](Widget*){ reply(P2PJoinRequestReply::Yes); });
  reader.registerCallback("no", [this](Widget*){ reply(P2PJoinRequestReply::No); });
  reader.registerCallback("ignore", [this](Widget*){ reply(P2PJoinRequestReply::Ignore); });

  m_confirmed = false;

  Json config = assets->json("/interface/windowconfig/joinrequest.config");

  reader.construct(config.get("paneLayout"), this);

  String message = config.getString("joinMessage").replaceTags(StringMap<String>{{"username", userName}});
  fetchChild<LabelWidget>("message")->setText(message);

  show();
}

void JoinRequestDialog::reply(P2PJoinRequestReply reply) {
  m_confirmed = true;
  m_callback(reply);
  dismiss();
}

void JoinRequestDialog::dismissed() {
  if (!m_confirmed)
    m_callback(P2PJoinRequestReply::No);

  Pane::dismissed();
}

}
