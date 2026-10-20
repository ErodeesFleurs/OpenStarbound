#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"

import std;
import star.format;
import star.string;
import star.list;
import star.json;

import star.mod_uploader;
import star.steam_dispatch;
import star.s_plain_text_edit;
import star.io_device;
import star.map;
import star.asset_source;
import star.directory_asset_source;
import star.file;
import star.thread;
import star.lexical_cast;
import star.string_conversion;
import star.packed_asset_source;
import star.logging;
import star.random;

namespace Star {

ModUploader::ModUploader(Logger& logger, RandomService& random)
  : QMainWindow(), m_logger(logger), m_random(random) {
  m_steamDispatch.initialize();
  connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, [this] {
    m_closing = true;
  });

  auto selectDirectoryButton = new QPushButton("Select Mod Directory");
  m_directoryLabel = new QLabel();
  m_reloadButton = new QPushButton("Reload");
  m_nameEditor = new QLineEdit();
  m_titleEditor = new QLineEdit();
  m_authorEditor = new QLineEdit();
  m_versionEditor = new QLineEdit();
  m_descriptionEditor = new SPlainTextEdit();
  m_previewImageLabel = new QLabel();
  auto selectPreviewImageButton = new QPushButton("Select");
  m_modIdLabel = new QLabel();
  auto resetModIdButton = new QPushButton("Reset Steam Mod Information");
  auto userAgreementLabel = new QLabel("By submitting this item, you agree to the <a href=\"http://steamcommunity.com/sharedfiles/workshoplegalagreement\">workshop terms of service</a>");
  auto uploadButton = new QPushButton("Upload to Steam!");

  m_categorySelectors.set("Armor and Clothes", new QCheckBox("Armor and Clothes"));
  m_categorySelectors.set("Character Improvements", new QCheckBox("Character Improvements"));
  m_categorySelectors.set("Cheats and God Items", new QCheckBox("Cheats and God Items"));
  m_categorySelectors.set("Crafting and Building", new QCheckBox("Crafting and Building"));
  m_categorySelectors.set("Dungeons", new QCheckBox("Dungeons"));
  m_categorySelectors.set("Food and Farming", new QCheckBox("Food and Farming"));
  m_categorySelectors.set("Furniture and Objects", new QCheckBox("Furniture and Objects"));
  m_categorySelectors.set("In-Game Tools", new QCheckBox("In-Game Tools"));
  m_categorySelectors.set("Mechanics", new QCheckBox("Mechanics"));
  m_categorySelectors.set("Miscellaneous", new QCheckBox("Miscellaneous"));
  m_categorySelectors.set("Musical Instruments and Songs", new QCheckBox("Musical Instruments and Songs"));
  m_categorySelectors.set("NPCs and Creatures", new QCheckBox("NPCs and Creatures"));
  m_categorySelectors.set("Planets and Environments", new QCheckBox("Planets and Environments"));
  m_categorySelectors.set("Quests", new QCheckBox("Quests"));
  m_categorySelectors.set("Species", new QCheckBox("Species"));
  m_categorySelectors.set("Ships", new QCheckBox("Ships"));
  m_categorySelectors.set("User Interface", new QCheckBox("User Interface"));
  m_categorySelectors.set("Vehicles and Mounts", new QCheckBox("Vehicles and Mounts"));
  m_categorySelectors.set("Weapons", new QCheckBox("Weapons"));

  m_modIdLabel->setOpenExternalLinks(true);
  userAgreementLabel->setOpenExternalLinks(true);

  connect(selectDirectoryButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));
  connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(loadDirectory()));
  connect(selectPreviewImageButton, SIGNAL(clicked()), this, SLOT(selectPreview()));
  connect(m_nameEditor, SIGNAL(editingFinished()), this, SLOT(writeMetadata()));
  connect(m_titleEditor, SIGNAL(editingFinished()), this, SLOT(writeMetadata()));
  connect(m_authorEditor, SIGNAL(editingFinished()), this, SLOT(writeMetadata()));
  connect(m_versionEditor, SIGNAL(editingFinished()), this, SLOT(writeMetadata()));
  connect(m_descriptionEditor, SIGNAL(editingFinished()), this, SLOT(writeMetadata()));
  connect(resetModIdButton, SIGNAL(clicked()), this, SLOT(resetModId()));
  connect(uploadButton, SIGNAL(clicked()), this, SLOT(uploadToSteam()));

  for (auto pair : m_categorySelectors) {
    connect(pair.second, SIGNAL(clicked()), this, SLOT(writeMetadata()));
  }

  auto loadDirectoryLayout = new QHBoxLayout();
  loadDirectoryLayout->addWidget(selectDirectoryButton);
  loadDirectoryLayout->addWidget(m_directoryLabel, 1);
  loadDirectoryLayout->addWidget(m_reloadButton);

  QGridLayout* editorLeftLayout = new QGridLayout();
  editorLeftLayout->addWidget(new QLabel("Name"), 0, 0);
  editorLeftLayout->addWidget(m_nameEditor, 0, 1, 1, 2);

  editorLeftLayout->addWidget(new QLabel("Title"), 1, 0);
  editorLeftLayout->addWidget(m_titleEditor, 1, 1, 1, 2);

  editorLeftLayout->addWidget(new QLabel("Author"), 2, 0);
  editorLeftLayout->addWidget(m_authorEditor, 2, 1, 1, 2);

  editorLeftLayout->addWidget(new QLabel("Version"), 3, 0);
  editorLeftLayout->addWidget(m_versionEditor, 3, 1, 1, 2);

  editorLeftLayout->addWidget(new QLabel("Description"), 4, 0);
  editorLeftLayout->addWidget(m_descriptionEditor, 4, 1, 1, 2);

  editorLeftLayout->addWidget(new QLabel("Preview Image"), 5, 0);
  editorLeftLayout->addWidget(m_previewImageLabel, 5, 1);
  editorLeftLayout->addWidget(selectPreviewImageButton, 5, 2);

  editorLeftLayout->addWidget(new QLabel("Mod ID"), 6, 0);
  editorLeftLayout->addWidget(m_modIdLabel, 6, 1);
  editorLeftLayout->addWidget(resetModIdButton, 6, 2);

  editorLeftLayout->addWidget(userAgreementLabel, 7, 0, 1, 3, Qt::AlignCenter);
  editorLeftLayout->addWidget(uploadButton, 8, 0, 1, 3);

  editorLeftLayout->setColumnStretch(1, 1);

  QVBoxLayout* categoryLayout = new QVBoxLayout();
  categoryLayout->addWidget(new QLabel("Categories"));
  auto categoryKeys = m_categorySelectors.keys();
  categoryKeys.sort();
  for (auto k : categoryKeys) {
    categoryLayout->addWidget(m_categorySelectors[k]);
  }
  categoryLayout-> addWidget(new QWidget(), 1);

  QHBoxLayout* editorLayout = new QHBoxLayout();
  editorLayout->addLayout(editorLeftLayout, 1);
  editorLayout->addLayout(categoryLayout);

  m_editorSection = new QWidget();
  m_editorSection->setLayout(editorLayout);

  auto mainLayout = new QVBoxLayout();
  mainLayout->addLayout(loadDirectoryLayout);
  mainLayout->addWidget(m_editorSection);

  auto* centralWidget = new QWidget(this);
  centralWidget->setLayout(mainLayout);
  setCentralWidget(centralWidget);

  m_reloadButton->setEnabled(false);
  m_editorSection->setEnabled(false);

  setWindowTitle("Steam Mod Uploader");
  resize(1000, 600);
}

void ModUploader::selectDirectory() {
  QString dir = QFileDialog::getExistingDirectory(this, "Select the top-level mod directory");
  m_modDirectory = toSString(dir);

  loadDirectory();
}

void ModUploader::loadDirectory() {
  QProgressDialog progress("Loading mod directory...", "", 0, 0, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setCancelButton(nullptr);
  progress.setAutoReset(false);
  progress.show();

  if (m_modDirectory && !File::isDirectory(*m_modDirectory))
    m_modDirectory.reset();

  if (!m_modDirectory) {
    m_reloadButton->setEnabled(false);
    m_directoryLabel->setText("");
    m_editorSection->setEnabled(false);
    m_assetSource.reset();
    return;
  }

  m_reloadButton->setEnabled(true);
  m_directoryLabel->setText(toQString(*m_modDirectory));
  m_editorSection->setEnabled(true);
  m_assetSource.emplace(DirectoryAssetSource(m_logger, *m_modDirectory));

  JsonObject metadata = m_assetSource->metadata();
  m_nameEditor->setText(toQString(metadata.value("name", "").toString()));
  m_titleEditor->setText(toQString(metadata.value("friendlyName", "").toString()));
  m_authorEditor->setText(toQString(metadata.value("author", "").toString()));
  m_versionEditor->setText(toQString(metadata.value("version", "").toString()));
  m_descriptionEditor->setPlainText(toQString(metadata.value("description", "").toString()));

  for (auto pair : m_categorySelectors)
    pair.second->setChecked(false);

  auto tagString = metadata.value("tags", "").toString();
  auto tagList = tagString.split('|');
  for (auto tag : tagList) {
    if (m_categorySelectors.contains(tag))
      m_categorySelectors[tag]->setChecked(true);
  }

  String modId = metadata.value("steamContentId", "").toString();
  m_modIdLabel->setText(toQString(strf("<a href=\"https://steamcommunity.com/sharedfiles/filedetails/?id={}\">{}</a>", modId, modId)));

  String previewFile = File::relativeTo(*m_modDirectory, "_previewimage");
  if (File::isFile(previewFile)) {
    m_modPreview = QImage(toQString(previewFile), "PNG");
    m_previewImageLabel->setPixmap(QPixmap::fromImage(m_modPreview));
  } else {
    m_modPreview = {};
    m_previewImageLabel->setPixmap({});
  }
}

void ModUploader::selectPreview() {
  QString image = QFileDialog::getOpenFileName(this, "Select a mod preview image", "", "Images (*.png *.jpg)");

  m_modPreview = {};
  m_previewImageLabel->setPixmap({});

  if (!image.isEmpty()) {
    if (m_modPreview.load(image))
      m_previewImageLabel->setPixmap(QPixmap::fromImage(m_modPreview));
    else
      QMessageBox::critical(this, "Error", "Could not load preview image");
  }

  writePreview();
}

void ModUploader::writeMetadata() {
  if (!m_assetSource)
    return;

  auto metadata = m_assetSource->metadata();
  auto setMetadata = [&metadata](String const& key, String const& value) {
    if (value.empty())
      metadata.remove(key);
    else
      metadata[key] = value;
  };

  setMetadata("name", toSString(m_nameEditor->text()));
  setMetadata("friendlyName", toSString(m_titleEditor->text()));
  setMetadata("author", toSString(m_authorEditor->text()));
  setMetadata("version", toSString(m_versionEditor->text()));
  setMetadata("description", toSString(m_descriptionEditor->toPlainText()));

  auto tagList = StringList();
  for (auto pair : m_categorySelectors) {
    if (pair.second->isChecked())
      tagList.append(pair.first);
  }
  auto tagString = tagList.join("|");
  setMetadata("tags", tagString);

  m_assetSource->setMetadata(metadata);
}

void ModUploader::writePreview() {
  if (m_modPreview.isNull())
    return;

  String modPreviewFile = File::relativeTo(*m_modDirectory, "_previewimage");
  m_modPreview.save(toQString(modPreviewFile), "PNG");
}

void ModUploader::resetModId() {
  m_modIdLabel->setText("");
  auto metadata = m_assetSource->metadata();
  metadata.remove("steamContentId");
  m_assetSource->setMetadata(metadata);
}

void ModUploader::closeEvent(QCloseEvent* event) {
  QMainWindow::closeEvent(event);
  if (event->isAccepted())
    m_closing = true;
}

void ModUploader::uploadToSteam() {
  if (!m_modDirectory || m_closing)
    return;

  QProgressDialog progress("Uploading to Steam...", "", 0, 0, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setCancelButton(nullptr);
  progress.setAutoReset(false);
  progress.show();

  if (m_assetSource->assetPaths().empty()) {
    QMessageBox::critical(this, "Error", "Cannot upload, mod has no content");
    return;
  }

  m_steamItemCreateResult = {};
  m_steamItemSubmitResult = {};

  JsonObject metadata = m_assetSource->metadata();
  String modIdString = metadata.value("steamContentId", "").toString();
  if (modIdString.empty()) {
    SteamCallResult<ModUploader, CreateItemResult_t> callResultCreate(m_steamDispatch,
        SteamAPI_ISteamUGC_CreateItem(SteamAPI_SteamUGC(),
            SteamAPI_ISteamUtils_GetAppID(SteamAPI_SteamUtils()), k_EWorkshopFileTypeCommunity),
        this, &ModUploader::onSteamCreateItem);

    progress.setLabelText("Creating new Steam UGC Item");
    while (!m_steamItemCreateResult && !m_closing) {
      QApplication::processEvents();
      if (!m_closing)
        m_steamDispatch.runCallbacks();
      Thread::sleep(20);
    }
    if (m_closing)
      return;

    if (m_steamItemCreateResult->second) {
      QMessageBox::critical(this, "Error", "There was an IO error creating a new Steam UGC item");
      return;
    }

    if (m_steamItemCreateResult->first.m_bUserNeedsToAcceptWorkshopLegalAgreement) {
      QMessageBox::critical(this, "Error", "The current Steam user has not agreed to the workshop legal agreement");
      return;
    }

    if (m_steamItemCreateResult->first.m_eResult == k_EResultInsufficientPrivilege) {
      QMessageBox::critical(this, "Error", "Insufficient privileges to create a new Steam UGC item");
      return;
    }

    if (m_steamItemCreateResult->first.m_eResult != k_EResultOK) {
      QMessageBox::critical(this, "Error", strf("Error creating new Steam UGC Item ({})", (int) m_steamItemCreateResult->first.m_eResult).c_str());
      return;
    }

    modIdString = toString(static_cast<PublishedFileId_t>(m_steamItemCreateResult->first.m_nPublishedFileId));
    String modUrl = strf("https://steamcommunity.com/sharedfiles/filedetails/?id={}", modIdString);

    metadata.set("steamContentId", modIdString);
    metadata.set("link", modUrl);
    m_assetSource->setMetadata(metadata);

    m_modIdLabel->setText(toQString(strf("<a href=\"{}\">{}</a>", modUrl, modIdString)));
  }

  String steamUploadDir = File::temporaryDirectory(m_random);
  auto progressCallback = [&progress](std::size_t i, std::size_t total, String, String assetPath) {
    progress.setLabelText(toQString(strf("Packing '{}'", assetPath)));
    progress.setMaximum(total);
    progress.setValue(i);
    QApplication::processEvents();
  };

  String packedPath = File::relativeTo(steamUploadDir, "contents.pak");
  PackedAssetSource::build(*m_assetSource, packedPath, {}, progressCallback);
  if (m_closing) {
    File::removeDirectoryRecursive(steamUploadDir);
    return;
  }

  PublishedFileId_t modId = lexicalCast<PublishedFileId_t>(modIdString);

  UGCUpdateHandle_t updateHandle = SteamAPI_ISteamUGC_StartItemUpdate(SteamAPI_SteamUGC(),
      SteamAPI_ISteamUtils_GetAppID(SteamAPI_SteamUtils()), modId);
  SteamAPI_ISteamUGC_SetItemTitle(SteamAPI_SteamUGC(), updateHandle, toSString(m_titleEditor->text()).utf8Ptr());
  SteamAPI_ISteamUGC_SetItemDescription(SteamAPI_SteamUGC(), updateHandle, toSString(m_descriptionEditor->toPlainText()).utf8Ptr());
  if (!m_modPreview.isNull())
    SteamAPI_ISteamUGC_SetItemPreview(SteamAPI_SteamUGC(), updateHandle, File::relativeTo(*m_modDirectory, "_previewimage").utf8Ptr());
  SteamAPI_ISteamUGC_SetItemContent(SteamAPI_SteamUGC(), updateHandle, steamUploadDir.utf8Ptr());

  // construct tags
  auto tagList = StringList();
  for (auto entry : m_categorySelectors.pairs()) {
    if (entry.second->isChecked())
      tagList.append(entry.first);
  }
  List<char const*> tagStrings(tagList.size(), nullptr);
  for (std::size_t i = 0; i < tagList.size(); ++i) {
    tagStrings[i] = tagList[i].utf8Ptr();
  }

  SteamParamStringArray_t itemTags = {tagStrings.ptr(), (std::int32_t)tagList.size()};
  SteamAPI_ISteamUGC_SetItemTags(SteamAPI_SteamUGC(), updateHandle, &itemTags, false);

  SteamCallResult<ModUploader, SubmitItemUpdateResult_t> callResultSubmit(m_steamDispatch,
      SteamAPI_ISteamUGC_SubmitItemUpdate(SteamAPI_SteamUGC(), updateHandle, nullptr),
      this, &ModUploader::onSteamSubmitItem);

  progress.setLabelText("Updating Steam UGC Item");
  while (!m_steamItemSubmitResult && !m_closing) {
    uint64 bytesProcessed;
    uint64 bytesTotal;
    SteamAPI_ISteamUGC_GetItemUpdateProgress(SteamAPI_SteamUGC(), updateHandle, &bytesProcessed, &bytesTotal);
    progress.setMaximum(bytesTotal);
    progress.setValue(bytesProcessed);
    QApplication::processEvents();
    if (!m_closing)
      m_steamDispatch.runCallbacks();
    Thread::sleep(20);
  }

  File::removeDirectoryRecursive(steamUploadDir);
  if (m_closing)
    return;

  if (m_steamItemSubmitResult->second) {
    QMessageBox::critical(this, "Error", "There was an IO error submitting changes to the Steam UGC item");
    return;
  }

  if (m_steamItemSubmitResult->first.m_bUserNeedsToAcceptWorkshopLegalAgreement) {
    QMessageBox::critical(this, "Error", "The current Steam user has not agreed to the workshop legal agreement");
    return;
  }

  if (m_steamItemSubmitResult->first.m_eResult != k_EResultOK) {
    QMessageBox::critical(this, "Error", strf("Error submitting changes to the Steam UGC item ({})", (int) m_steamItemSubmitResult->first.m_eResult).c_str());
    return;
  }
}

void ModUploader::onSteamCreateItem(CreateItemResult_t* result, bool ioFailure) {
  m_steamItemCreateResult = std::make_pair(*result, ioFailure);
}

void ModUploader::onSteamSubmitItem(SubmitItemUpdateResult_t* result, bool ioFailure) {
  m_steamItemSubmitResult = std::make_pair(*result, ioFailure);
}

}

// Generate with moc -i from StarModUploader.cppm; compile in this scanned owner.
#include "moc_StarModUploader.cpp"
