module;
#include "StarString.hpp"
#include "StarByteArray.hpp"
import star.encode;
#include "StarBytes.hpp"
#include "StarFormat.hpp"
#include "StarThread.hpp"
import star.time;
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarInputEvent.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarVector.hpp"
import star.text;
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVector.hpp"
#include "StarMaybe.hpp"
#include "StarGameTypes.hpp"
#include "StarCasting.hpp"
import star.listener;
#include "StarOrderedMap.hpp"

import star.shell_parser;
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"

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
STAR_CLASS(UniverseClient);
STAR_CLASS(Cinematic);
}

namespace Star {

STAR_CLASS(Quest);

class ClientCommandProcessor {
public:
  ClientCommandProcessor(UniverseClientPtr universeClient, CinematicPtr cinematicOverlay,
      MainInterfacePaneManager* paneManager, StringMap<StringList> macroCommands);

  StringList handleCommand(String const& commandLine, bool userInput = false);
  StringList updatePromises();

  bool debugDisplayEnabled() const;
  bool debugHudEnabled() const;
  bool fixedCameraEnabled() const;

private:
  bool adminCommandAllowed() const;
  String previewQuestPane(StringList const& arguments, function<PanePtr(QuestPtr)> createPane);

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

  UniverseClientPtr m_universeClient;
  CinematicPtr m_cinematicOverlay;
  MainInterfacePaneManager* m_paneManager;
  CaseInsensitiveStringMap<function<String(String const&)>> m_builtinCommands;
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
  using ::Star::QuestPtr;
  using ::Star::QuestConstPtr;
  using ::Star::QuestWeakPtr;
  using ::Star::QuestConstWeakPtr;
  using ::Star::QuestUPtr;
  using ::Star::QuestConstUPtr;
  using ::Star::ClientCommandProcessor;
}
