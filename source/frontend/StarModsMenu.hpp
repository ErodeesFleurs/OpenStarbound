#pragma once

#include "StarAssets.hpp"
#include "StarPane.hpp"

namespace Star {

class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;

class ModsMenu : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
    GuiContext& guiContext;
  };

  explicit ModsMenu(Services services);

  void update(float dt) override;

private:
  [[nodiscard]] static String bestModName(JsonObject const& metadata, String const& sourcePath);

  void openLink();
  void openWorkshop();

  StringList m_assetsSources;
  AssetsConstPtr m_assets;

  WidgetRef<ListWidget> m_modList;
  WidgetRef<LabelWidget> m_modName;
  WidgetRef<LabelWidget> m_modAuthor;
  WidgetRef<LabelWidget> m_modVersion;
  WidgetRef<LabelWidget> m_modPath;
  WidgetRef<LabelWidget> m_modDescription;

  WidgetRef<ButtonWidget> m_linkButton;
  WidgetRef<ButtonWidget> m_copyLinkButton;
};

}
