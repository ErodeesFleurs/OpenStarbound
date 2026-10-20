import std;
import star.memory;
import star.json;
import star.casting;
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
import star.file;
import star.ref_ptr;
import star.logging;
import star.version;
import star.string;
import star.text;
import star.asset_path;
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


import star.mods_menu;
import star.widget_parsing;
import star.gui_reader;
import star.label_widget;
import star.button_group;
import star.button_widget;
import star.list_widget;
import star.image_widget;

namespace Star {

ModsMenu::ModsMenu(GuiContext& contextRef) : Pane(contextRef) {
  auto assets = context()->root().assets();

  GuiReader reader(*context());
  reader.registerCallback("linkbutton", std::bind(&ModsMenu::openLink, this));
  reader.registerCallback("workshopbutton", std::bind(&ModsMenu::openWorkshop, this));
  reader.construct(assets->json("/interface/modsmenu/modsmenu.config:paneLayout"), this);

  m_assetsSources = assets->assetSources();
  m_modList = fetchChild<ListWidget>("mods.list");
  for (auto const& assetsSource : m_assetsSources) {
    auto metadata = assets->assetSourceMetadata(assetsSource);
    auto listItem = m_modList->addItem();
    auto modName = listItem->fetchChild<LabelWidget>("name");
    modName->setText(bestModName(metadata, assetsSource));
    if (auto iconImage = metadata.ptr("icon")) {
      auto modIcon = listItem->fetchChild<ImageWidget>("icon");
      modIcon->setImage(iconImage->toString());
    }
  }

  m_modName = findChild<LabelWidget>("modname");
  m_modAuthor = findChild<LabelWidget>("modauthor");
  m_modVersion = findChild<LabelWidget>("modversion");
  m_modPath = findChild<LabelWidget>("modpath");
  m_modDescription = findChild<LabelWidget>("moddescription");

  m_linkButton = fetchChild<ButtonWidget>("linkbutton");
  m_copyLinkButton = fetchChild<ButtonWidget>("copylinkbutton");

  auto linkLabel = fetchChild<LabelWidget>("linklabel");
  auto copyLinkLabel = fetchChild<LabelWidget>("copylinklabel");
  auto workshopLinkButton = fetchChild<ButtonWidget>("workshopbutton");

  auto& guiContext = *context();
  bool hasDesktopService = (bool)guiContext.applicationController()->desktopService();

  workshopLinkButton->setEnabled(hasDesktopService);

  m_linkButton->setVisibility(hasDesktopService);
  m_copyLinkButton->setVisibility(!hasDesktopService);

  m_linkButton->setEnabled(false);
  m_copyLinkButton->setEnabled(false);

  linkLabel->setVisibility(hasDesktopService);
  copyLinkLabel->setVisibility(!hasDesktopService);
}

void ModsMenu::update(float dt) {
  Pane::update(dt);

  std::size_t selectedItem = m_modList->selectedItem();
  if (selectedItem == NPos) {
    m_modName->setText("");
    m_modAuthor->setText("");
    m_modVersion->setText("");
    m_modPath->setText("");
    m_modDescription->setText("");

  } else {
    String assetsSource = m_assetsSources.at(selectedItem);
    JsonObject assetsSourceMetadata = context()->root().assets()->assetSourceMetadata(assetsSource);

    m_modName->setText(bestModName(assetsSourceMetadata, assetsSource));
    m_modAuthor->setText(assetsSourceMetadata.value("author", "No Author Set").toString());
    m_modVersion->setText(assetsSourceMetadata.value("version", "No Version Set").printString());
    m_modPath->setText(assetsSource);
    m_modDescription->setText(assetsSourceMetadata.value("description", "").toString());

    String link = assetsSourceMetadata.value("link", "").toString();

    m_linkButton->setEnabled(!link.empty());
    m_copyLinkButton->setEnabled(!link.empty());
  }
}

String ModsMenu::bestModName(JsonObject const& metadata, String const& sourcePath) {
  if (auto ptr = metadata.ptr("friendlyName"))
    return ptr->toString();
  if (auto ptr = metadata.ptr("name"))
    return ptr->toString();
  String baseName = File::baseName(sourcePath);
  if (baseName.contains("."))
    baseName.rextract(".");
  return baseName;
}

void ModsMenu::openLink() {
  std::size_t selectedItem = m_modList->selectedItem();
  if (selectedItem == NPos)
    return;

  String assetsSource = m_assetsSources.at(selectedItem);
  JsonObject assetsSourceMetadata = context()->root().assets()->assetSourceMetadata(assetsSource);
  String link = assetsSourceMetadata.value("link", "").toString();

  if (link.empty())
    return;

  auto& guiContext = *context();
  if (auto desktopService = guiContext.applicationController()->desktopService())
    desktopService->openUrl(link);
  else
    guiContext.setClipboard(link);
}

void ModsMenu::openWorkshop() {
  auto assets = context()->root().assets();
  auto& guiContext = *context();
  if (auto desktopService = guiContext.applicationController()->desktopService())
    desktopService->openUrl(assets->json("/interface/modsmenu/modsmenu.config:workshopLink").toString());
}

}
