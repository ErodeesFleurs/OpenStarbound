#pragma once

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
  ModsMenu();

  void update(float dt) override;

private:
  static String bestModName(JsonObject const& metadata, String const& sourcePath);

  void openLink();
  void openWorkshop();

  StringList m_assetsSources;

  ListWidgetPtr m_modList;
  LabelWidgetPtr m_modName;
  LabelWidgetPtr m_modAuthor;
  LabelWidgetPtr m_modVersion;
  LabelWidgetPtr m_modPath;
  LabelWidgetPtr m_modDescription;

  ButtonWidgetPtr m_linkButton;
  ButtonWidgetPtr m_copyLinkButton;
};

}
