module;
import std;
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
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.rpc_promise;


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

enum class P2PJoinRequestReply;

class JoinRequestDialog : public Pane {
public:
  JoinRequestDialog(GuiContext& guiContext);

  virtual ~JoinRequestDialog() {}

  void displayRequest(String const& userName, std::function<void(P2PJoinRequestReply)> callback);

  void dismissed() override;

private:
  void reply(P2PJoinRequestReply reply);

  std::function<void(P2PJoinRequestReply)> m_callback;
  bool m_confirmed;
};

}

export module star.join_request_dialog;

export namespace Star {
  using ::Star::JoinRequestDialog;
}
