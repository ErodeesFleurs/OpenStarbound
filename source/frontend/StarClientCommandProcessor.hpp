#pragma once

#include "StarShellParser.hpp"
#include "StarAssets.hpp"
#include "StarConfiguration.hpp"
#include "StarItemDatabase.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaRoot.hpp"
#include "StarUniverseClient.hpp"
#include "StarQuestManager.hpp"
#include "StarCinematic.hpp"
#include "StarMainInterfaceTypes.hpp"

namespace Star {

struct FramesSpecification;
using FramesSpecificationConstPtr = SharedPtr<FramesSpecification const>;
class ByteArray;
class Image;
class Input;
class GuiContext;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

struct ClientCommandProcessorServices {
  ClientCommandProcessorServices(GuiContext& guiContext, Input& input);

  AssetsConstPtr assets;
  ConfigurationPtr configuration;
  ItemDatabaseConstPtr itemDatabase;
  ObjectDatabaseConstPtr objectDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
  function<FramesSpecificationConstPtr(String const&)> imageFrames;
  String outputDirectory;
  function<void()> reloadRoot;
  function<void()> hotReloadRoot;
  GuiContext& guiContext;
  Input& input;
  function<bool(Image const&, ByteArray*, String const*)> setClipboardImage;
};

class ClientCommandProcessor {
public:
  ClientCommandProcessor(UniverseClientPtr universeClient, CinematicPtr cinematicOverlay,
      MainInterfacePaneManager& paneManager, StringMap<StringList> macroCommands, ClientCommandProcessorServices services);

  StringList handleCommand(String const& commandLine, bool userInput = false);

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
  MainInterfacePaneManager& m_paneManager;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  function<FramesSpecificationConstPtr(String const&)> m_imageFrames;
  String m_outputDirectory;
  function<void()> m_reloadRoot;
  function<void()> m_hotReloadRoot;
  GuiContext& m_guiContext;
  Input& m_input;
  function<bool(Image const&, ByteArray*, String const*)> m_setClipboardImage;
  CaseInsensitiveStringMap<function<String(String const&)>> m_builtinCommands;
  StringMap<StringList> m_macroCommands;
  ShellParser m_parser;
  LuaBaseComponent m_scriptComponent;
  bool m_debugDisplayEnabled = false;
  bool m_debugHudEnabled = true;
  bool m_fixedCameraEnabled = false;
};

}
