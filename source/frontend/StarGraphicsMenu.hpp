#pragma once

#include "StarIAssets.hpp"
#include "StarIConfiguration.hpp"
#include "StarPane.hpp"
#include "StarMainInterfaceTypes.hpp"
#include "StarUniverseClient.hpp"

namespace Star {

class GraphicsMenu;
using GraphicsMenuPtr = SharedPtr<GraphicsMenu>;
class ShadersMenu;
using ShadersMenuPtr = SharedPtr<ShadersMenu>;

struct GraphicsMenuServices {
  IAssetsConstPtr assets;
  IConfigurationPtr configuration;
};

class GraphicsMenu : public Pane {
public:
  GraphicsMenu(PaneManager* manager, UniverseClientPtr client, GraphicsMenuServices services);

  void show() override;
  void dismissed() override;

  void toggleFullscreen();

private:
  static StringList const ConfigKeys;

  void initConfig();
  void syncGui();

  void apply();
  void applyWindowSettings();
  
  void displayShaders();

  List<Vec2U> m_resList;
  List<float> m_interfaceScaleList;
  List<float> m_zoomList;
  List<float> m_cameraSpeedList;

  JsonObject m_localChanges;
  
  ShadersMenuPtr m_shadersMenu;
  PaneManager* m_paneManager;
  IAssetsConstPtr m_assets;
  IConfigurationPtr m_configuration;
};

}
