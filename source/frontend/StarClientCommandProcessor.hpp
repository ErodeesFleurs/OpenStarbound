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

  [[nodiscard]] StringList handleCommand(String const& commandLine, bool userInput = false);

  [[nodiscard]] bool debugDisplayEnabled() const;
  [[nodiscard]] bool debugHudEnabled() const;
  [[nodiscard]] bool fixedCameraEnabled() const;

private:
  [[nodiscard]] bool adminCommandAllowed() const;
  [[nodiscard]] String previewQuestPane(StringList const& arguments, function<PanePtr(QuestPtr)> createPane);

  [[nodiscard]] String reload();
  [[nodiscard]] String hotReload();
  [[nodiscard]] String whoami();
  [[nodiscard]] String gravity();
  [[nodiscard]] String debug(String const& argumentsString);
  [[nodiscard]] String boxes();
  [[nodiscard]] String fullbright();
  [[nodiscard]] String asyncLighting();
  [[nodiscard]] String setGravity(String const& argumentsString);
  [[nodiscard]] String resetGravity();
  [[nodiscard]] String fixedCamera();
  [[nodiscard]] String monochromeLighting();
  [[nodiscard]] String radioMessage(String const& argumentsString);
  [[nodiscard]] String clearRadioMessages();
  [[nodiscard]] String clearCinematics();
  [[nodiscard]] String startQuest(String const& argumentsString);
  [[nodiscard]] String completeQuest(String const& argumentsString);
  [[nodiscard]] String failQuest(String const& argumentsString);
  [[nodiscard]] String previewNewQuest(String const& argumentsString);
  [[nodiscard]] String previewQuestComplete(String const& argumentsString);
  [[nodiscard]] String previewQuestFailed(String const& argumentsString);
  [[nodiscard]] String clearScannedObjects();
  [[nodiscard]] String playTime();
  [[nodiscard]] String deathCount();
  [[nodiscard]] String cinema(String const& argumentsString);
  [[nodiscard]] String suicide();
  [[nodiscard]] String naked();
  [[nodiscard]] String resetAchievements();
  [[nodiscard]] String statistic(String const& argumentsString);
  [[nodiscard]] String giveEssentialItem(String const& argumentsString);
  [[nodiscard]] String makeTechAvailable(String const& argumentsString);
  [[nodiscard]] String enableTech(String const& argumentsString);
  [[nodiscard]] String upgradeShip(String const& argumentsString);
  [[nodiscard]] String swap(String const& argumentsString);
  [[nodiscard]] String respawnInWorld(String const& argumentsString);
  [[nodiscard]] String render(String const& imagePath);

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
