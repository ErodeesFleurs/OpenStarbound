module;
import star.uuid;
import std;
import star.json;
import star.string;
import star.byte_array;
import star.encode;
import star.bytes;
import star.format;
import star.thread;
import star.time;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.input_event;
import star.bi_map;
import star.text;
import star.audio;
import star.map;
import star.set;
import star.maybe;
import star.rpc_promise;
import star.game_types;
import star.casting;
import star.listener;
import star.ordered_map;

import star.shell_parser;
import star.lua_components;
import star.lua_root;

import star.drawable;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.mixer;

import star.cinematic;

import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.gui_context;
import star.widget;
import star.pane;
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;

import star.main_interface_types;

namespace Star {
class UniverseClient;
class Cinematic;
}

namespace Star {

class Quest;

class ClientCommandProcessor {
public:
  ClientCommandProcessor(std::shared_ptr<UniverseClient> universeClient, std::shared_ptr<Cinematic> cinematicOverlay,
      MainInterfacePaneManager* paneManager, StringMap<StringList> macroCommands);

  StringList handleCommand(String const& commandLine, bool userInput = false);
  StringList updatePromises();

  bool debugDisplayEnabled() const;
  bool debugHudEnabled() const;
  bool fixedCameraEnabled() const;

private:
  bool adminCommandAllowed() const;
  String previewQuestPane(StringList const& arguments, std::function<std::shared_ptr<Pane>(std::shared_ptr<Quest>)> createPane);

  String reload();
  String hotReload();
  String whoami();
  String gravity();
  String debug(String const& argumentsString);
  String boxes();
  String fullbright();
  String asyncLighting();
  String setGravity(String const& argumentsString);
  String resetGravity();
  String fixedCamera();
  String monochromeLighting();
  String radioMessage(String const& argumentsString);
  String clearRadioMessages();
  String clearCinematics();
  String startQuest(String const& argumentsString);
  String completeQuest(String const& argumentsString);
  String failQuest(String const& argumentsString);
  String previewNewQuest(String const& argumentsString);
  String previewQuestComplete(String const& argumentsString);
  String previewQuestFailed(String const& argumentsString);
  String clearScannedObjects();
  String playTime();
  String deathCount();
  String cinema(String const& argumentsString);
  String suicide();
  String naked();
  String resetAchievements();
  String statistic(String const& argumentsString);
  String giveEssentialItem(String const& argumentsString);
  String makeTechAvailable(String const& argumentsString);
  String enableTech(String const& argumentsString);
  String upgradeShip(String const& argumentsString);
  String swap(String const& argumentsString);
  String respawnInWorld(String const& argumentsString);
  String render(String const& imagePath);

  std::shared_ptr<UniverseClient> m_universeClient;
  std::shared_ptr<Cinematic> m_cinematicOverlay;
  MainInterfacePaneManager* m_paneManager;
  CaseInsensitiveStringMap<std::function<String(String const&)>> m_builtinCommands;
  StringMap<StringList> m_macroCommands;
  HashMap<Uuid,RpcPromise<Json>> m_commandPromises;
  ShellParser m_parser;
  bool m_debugDisplayEnabled = false;
  bool m_debugHudEnabled = true;
  bool m_fixedCameraEnabled = false;
};

}

export module star.client_command_processor;

export namespace Star {
  using ::Star::Quest;
  using ::Star::ClientCommandProcessor;
}
