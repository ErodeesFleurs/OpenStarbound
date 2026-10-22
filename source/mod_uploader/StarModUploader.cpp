#include <QApplication>
#include <QBuffer>
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
import star.output_proxy;
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
std::expected<std::unique_ptr<ModUploader>, StarException> ModUploader::create(Logger& logger, RandomService& random) {
  auto uploader = std::unique_ptr<ModUploader>(new ModUploader(logger, random));
  auto initialized = uploader->m_steamDispatch.initialize();
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  return uploader;
}

void ModUploader::showError(StarException const& error) {
  QMessageBox::critical(this, "Error", toQString(String(strf("{}", outputException(ExceptionDiagnostic(error), true)))));
}


ModUploader::ModUploader(Logger& logger, RandomService& random)
  : QMainWindow(), m_logger(logger), m_random(random) {
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
  if (dir.isEmpty())
    return;
  m_modDirectory = toSString(dir);
  loadDirectory();
}

void ModUploader::loadDirectory() {
  QProgressDialog progress("Loading mod directory...", "", 0, 0, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setCancelButton(nullptr);
  progress.setAutoReset(false);
  progress.show();

  auto loaded = loadDirectoryImpl();
  if (!loaded) {
    m_assetSource.reset();
    m_editorSection->setEnabled(false);
    showError(loaded.error());
  }
}

std::expected<void, StarException> ModUploader::loadDirectoryImpl() {
  if (m_modDirectory) {
    auto directory = File::isDirectory(*m_modDirectory);
    if (!directory)
      return std::unexpected(std::move(directory.error()));
    if (!*directory)
      m_modDirectory.reset();
  }

  m_reloadButton->setEnabled(m_modDirectory.has_value());
  m_directoryLabel->setText(m_modDirectory ? toQString(*m_modDirectory) : QString());
  if (!m_modDirectory) {
    m_editorSection->setEnabled(false);
    m_assetSource.reset();
    return {};
  }

  auto source = DirectoryAssetSource::create(m_logger, *m_modDirectory);
  if (!source)
    return std::unexpected(std::move(source.error()));

  auto metadata = (*source)->metadata();
  std::array<String, 7> fields;
  std::array<char const*, 7> keys = {"name", "friendlyName", "author", "version", "description", "tags", "steamContentId"};
  for (std::size_t i = 0; i < keys.size(); ++i) {
    auto field = metadata.value(keys[i], "").toString();
    if (!field)
      return std::unexpected(StarException(strf("Invalid mod metadata field '{}'", keys[i]), ExceptionDiagnostic(field.error())));
    fields[i] = std::move(*field);
  }
  auto tags = fields[5].split('|');
  if (!tags)
    return std::unexpected(std::move(tags.error()));

  // Commit the source and editors only after all metadata has been validated.
  m_assetSource = std::move(*source);
  m_nameEditor->setText(toQString(fields[0]));
  m_titleEditor->setText(toQString(fields[1]));
  m_authorEditor->setText(toQString(fields[2]));
  m_versionEditor->setText(toQString(fields[3]));
  m_descriptionEditor->setPlainText(toQString(fields[4]));
  for (auto const& pair : m_categorySelectors) {
    auto selected = tags->contains(pair.first);
    if (!selected)
      return std::unexpected(std::move(selected.error()));
    pair.second->setChecked(*selected);
  }

  m_modIdLabel->setText(toQString(String(strf("<a href=\"https://steamcommunity.com/sharedfiles/filedetails/?id={}\">{}</a>", fields[6], fields[6]))));
  String previewFile = File::relativeTo(*m_modDirectory, "_previewimage");
  auto previewExists = File::isFile(previewFile);
  if (!previewExists)
    return std::unexpected(std::move(previewExists.error()));
  if (*previewExists) {
    QImage preview(toQString(previewFile), "PNG");
    if (preview.isNull())
      return std::unexpected(StarException("Could not load mod preview image"));
    m_modPreview = std::move(preview);
  } else {
    m_modPreview = {};
  }
  m_previewImageLabel->setPixmap(QPixmap::fromImage(m_modPreview));
  m_editorSection->setEnabled(true);
  return {};
}

void ModUploader::selectPreview() {
  QString image = QFileDialog::getOpenFileName(this, "Select a mod preview image", "", "Images (*.png *.jpg)");
  if (image.isEmpty())
    return;

  QImage preview(image);
  if (preview.isNull()) {
    showError(StarException("Could not load preview image"));
    return;
  }
  auto saved = savePreview(preview);
  if (!saved) {
    showError(saved.error());
    return;
  }
  m_modPreview = std::move(preview);
  m_previewImageLabel->setPixmap(QPixmap::fromImage(m_modPreview));
}

void ModUploader::writeMetadata() {
  auto written = writeMetadataImpl();
  if (!written)
    showError(written.error());
}

std::expected<void, StarException> ModUploader::writeMetadataImpl() {
  if (!m_assetSource)
    return {};

  auto metadata = m_assetSource->metadata();
  auto setMetadata = [&metadata](String const& key, String const& value) {
    if (value.empty())
      metadata.remove(key);
    else
      metadata.set(key, value);
  };
  setMetadata("name", toSString(m_nameEditor->text()));
  setMetadata("friendlyName", toSString(m_titleEditor->text()));
  setMetadata("author", toSString(m_authorEditor->text()));
  setMetadata("version", toSString(m_versionEditor->text()));
  setMetadata("description", toSString(m_descriptionEditor->toPlainText()));

  StringList tags;
  for (auto const& pair : m_categorySelectors) {
    if (pair.second->isChecked())
      tags.append(pair.first);
  }
  setMetadata("tags", tags.join("|"));
  return m_assetSource->setMetadata(std::move(metadata));
}

std::expected<void, StarException> ModUploader::savePreview(QImage const& preview) {
  if (!m_modDirectory || !m_assetSource)
    return std::unexpected(StarException("Select a mod directory before saving a preview image"));

  QByteArray encoded;
  QBuffer buffer(&encoded);
  if (!buffer.open(QIODevice::WriteOnly) || !preview.save(&buffer, "PNG"))
    return std::unexpected(StarException("Could not encode preview image"));
  buffer.close();
  return File::overwriteFileWithRename(encoded.constData(), static_cast<std::size_t>(encoded.size()),
      File::relativeTo(*m_modDirectory, "_previewimage"));
}

void ModUploader::resetModId() {
  auto reset = resetModIdImpl();
  if (!reset)
    showError(reset.error());
}

std::expected<void, StarException> ModUploader::resetModIdImpl() {
  if (!m_assetSource)
    return {};
  auto metadata = m_assetSource->metadata();
  metadata.remove("steamContentId");
  auto written = m_assetSource->setMetadata(std::move(metadata));
  if (!written)
    return std::unexpected(std::move(written.error()));
  m_modIdLabel->setText("");
  return {};
}

void ModUploader::closeEvent(QCloseEvent* event) {
  QMainWindow::closeEvent(event);
  if (event->isAccepted())
    m_closing = true;
}

void ModUploader::uploadToSteam() {
  if (!m_modDirectory || !m_assetSource || m_closing)
    return;

  QProgressDialog progress("Uploading to Steam...", "", 0, 0, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setCancelButton(nullptr);
  progress.setAutoReset(false);
  progress.show();

  auto uploaded = uploadToSteamImpl(progress);
  if (!uploaded)
    showError(uploaded.error());
}

std::expected<void, StarException> ModUploader::uploadToSteamImpl(QProgressDialog& progress) {
  if (!SteamAPI_SteamUGC() || !SteamAPI_SteamUtils())
    return std::unexpected(StarException("Cannot upload: Steam is unavailable. Make sure Steam is running and online."));
  if (m_assetSource->assetPaths().empty())
    return std::unexpected(StarException("Cannot upload, mod has no content"));

  m_steamItemCreateResult.reset();
  m_steamItemSubmitResult.reset();
  auto metadata = m_assetSource->metadata();
  auto modIdField = metadata.value("steamContentId", "").toString();
  if (!modIdField)
    return std::unexpected(std::move(modIdField.error()));
  String modIdString = std::move(*modIdField);
  if (modIdString.empty()) {
    auto call = SteamAPI_ISteamUGC_CreateItem(SteamAPI_SteamUGC(),
        SteamAPI_ISteamUtils_GetAppID(SteamAPI_SteamUtils()), k_EWorkshopFileTypeCommunity);
    if (call == k_uAPICallInvalid)
      return std::unexpected(StarException("Could not create a new Steam UGC item. Make sure Steam is online."));
    SteamCallResult<ModUploader, CreateItemResult_t> callResultCreate(m_steamDispatch,
        call, this, &ModUploader::onSteamCreateItem);

    progress.setLabelText("Creating new Steam UGC Item");
    while (!m_steamItemCreateResult && !m_closing) {
      QApplication::processEvents();
      if (!m_closing) {
        auto dispatched = m_steamDispatch.runCallbacks();
        if (!dispatched)
          return std::unexpected(std::move(dispatched.error()));
      }
      Thread::sleep(20);
    }
    if (m_closing)
      return {};
    auto const& [result, ioFailure] = *m_steamItemCreateResult;
    if (ioFailure)
      return std::unexpected(StarException("There was an IO error creating a new Steam UGC item"));
    if (result.m_bUserNeedsToAcceptWorkshopLegalAgreement)
      return std::unexpected(StarException("The current Steam user has not agreed to the workshop legal agreement"));
    if (result.m_eResult == k_EResultInsufficientPrivilege)
      return std::unexpected(StarException("Insufficient privileges to create a new Steam UGC item"));
    if (result.m_eResult != k_EResultOK)
      return std::unexpected(StarException(strf("Error creating new Steam UGC Item ({})", static_cast<int>(result.m_eResult))));

    modIdString = toString(static_cast<PublishedFileId_t>(result.m_nPublishedFileId));
    String modUrl = strf("https://steamcommunity.com/sharedfiles/filedetails/?id={}", modIdString);
    metadata.set("steamContentId", modIdString);
    metadata.set("link", modUrl);
    auto written = m_assetSource->setMetadata(std::move(metadata));
    if (!written)
      return std::unexpected(std::move(written.error()));
    m_modIdLabel->setText(toQString(String(strf("<a href=\"{}\">{}</a>", modUrl, modIdString))));
  }

  auto modId = lexicalCast<PublishedFileId_t>(modIdString);
  if (!modId)
    return std::unexpected(std::move(modId.error()));
  auto temporary = File::temporaryDirectory(m_random);
  if (!temporary)
    return std::unexpected(std::move(temporary.error()));
  String steamUploadDir = std::move(*temporary);
  auto upload = [&]() -> std::expected<void, StarException> {
    auto progressCallback = [&progress](std::size_t i, std::size_t total, String, String assetPath) {
      progress.setLabelText(toQString(String(strf("Packing '{}'", assetPath))));
      progress.setMaximum(static_cast<int>(std::min(total, static_cast<std::size_t>(std::numeric_limits<int>::max()))));
      progress.setValue(static_cast<int>(std::min(i, static_cast<std::size_t>(std::numeric_limits<int>::max()))));
      QApplication::processEvents();
    };
    auto packed = PackedAssetSource::build(*m_assetSource,
        File::relativeTo(steamUploadDir, "contents.pak"), {}, progressCallback);
    if (!packed)
      return std::unexpected(std::move(packed.error()));
    if (m_closing)
      return {};

    UGCUpdateHandle_t updateHandle = SteamAPI_ISteamUGC_StartItemUpdate(SteamAPI_SteamUGC(),
        SteamAPI_ISteamUtils_GetAppID(SteamAPI_SteamUtils()), *modId);
    if (updateHandle == k_UGCUpdateHandleInvalid)
      return std::unexpected(StarException("Could not start Steam UGC item update"));
    if (!SteamAPI_ISteamUGC_SetItemTitle(SteamAPI_SteamUGC(), updateHandle, toSString(m_titleEditor->text()).utf8Ptr()))
      return std::unexpected(StarException("Steam rejected the item title"));
    if (!SteamAPI_ISteamUGC_SetItemDescription(SteamAPI_SteamUGC(), updateHandle, toSString(m_descriptionEditor->toPlainText()).utf8Ptr()))
      return std::unexpected(StarException("Steam rejected the item description"));
    if (!m_modPreview.isNull() && !SteamAPI_ISteamUGC_SetItemPreview(SteamAPI_SteamUGC(), updateHandle,
        File::relativeTo(*m_modDirectory, "_previewimage").utf8Ptr()))
      return std::unexpected(StarException("Steam rejected the preview image"));
    if (!SteamAPI_ISteamUGC_SetItemContent(SteamAPI_SteamUGC(), updateHandle, steamUploadDir.utf8Ptr()))
      return std::unexpected(StarException("Steam rejected the packed mod content"));

    StringList tags;
    for (auto const& entry : m_categorySelectors) {
      if (entry.second->isChecked())
        tags.append(entry.first);
    }
    List<char const*> tagStrings(tags.size(), nullptr);
    for (std::size_t i = 0; i < tags.size(); ++i)
      tagStrings[i] = tags[i].utf8Ptr();
    SteamParamStringArray_t itemTags = {tagStrings.ptr(), static_cast<std::int32_t>(tags.size())};
    if (!SteamAPI_ISteamUGC_SetItemTags(SteamAPI_SteamUGC(), updateHandle, &itemTags, false))
      return std::unexpected(StarException("Steam rejected the item tags"));

    auto call = SteamAPI_ISteamUGC_SubmitItemUpdate(SteamAPI_SteamUGC(), updateHandle, nullptr);
    if (call == k_uAPICallInvalid)
      return std::unexpected(StarException("Could not submit changes to the Steam UGC item. Make sure Steam is online."));
    SteamCallResult<ModUploader, SubmitItemUpdateResult_t> callResultSubmit(m_steamDispatch,
        call, this, &ModUploader::onSteamSubmitItem);
    progress.setLabelText("Updating Steam UGC Item");
    while (!m_steamItemSubmitResult && !m_closing) {
      uint64 bytesProcessed = 0;
      uint64 bytesTotal = 0;
      SteamAPI_ISteamUGC_GetItemUpdateProgress(SteamAPI_SteamUGC(), updateHandle, &bytesProcessed, &bytesTotal);
      auto maximum = static_cast<uint64>(std::numeric_limits<int>::max());
      progress.setMaximum(static_cast<int>(std::min(bytesTotal, maximum)));
      progress.setValue(static_cast<int>(std::min(bytesProcessed, maximum)));
      QApplication::processEvents();
      if (!m_closing) {
        auto dispatched = m_steamDispatch.runCallbacks();
        if (!dispatched)
          return std::unexpected(std::move(dispatched.error()));
      }
      Thread::sleep(20);
    }
    if (m_closing)
      return {};
    auto const& [result, ioFailure] = *m_steamItemSubmitResult;
    if (ioFailure)
      return std::unexpected(StarException("There was an IO error submitting changes to the Steam UGC item"));
    if (result.m_bUserNeedsToAcceptWorkshopLegalAgreement)
      return std::unexpected(StarException("The current Steam user has not agreed to the workshop legal agreement"));
    if (result.m_eResult != k_EResultOK)
      return std::unexpected(StarException(strf("Error submitting changes to the Steam UGC item ({})", static_cast<int>(result.m_eResult))));
    return {};
  }();
  auto cleanup = File::removeDirectoryRecursive(steamUploadDir);
  if (!cleanup) {
    if (!upload)
      return std::unexpected(StarException(strf("{}\nAdditionally failed to remove upload directory '{}': {}",
          upload.error().what(), steamUploadDir, cleanup.error().what()), ExceptionDiagnostic(upload.error())));
    return std::unexpected(StarException(strf("Failed to remove upload directory '{}'", steamUploadDir),
        ExceptionDiagnostic(cleanup.error())));
  }
  return upload;
}

std::expected<void, StarException> ModUploader::onSteamCreateItem(CreateItemResult_t* result, bool ioFailure) {
  m_steamItemCreateResult = std::make_pair(*result, ioFailure);
  return {};
}

std::expected<void, StarException> ModUploader::onSteamSubmitItem(SubmitItemUpdateResult_t* result, bool ioFailure) {
  m_steamItemSubmitResult = std::make_pair(*result, ioFailure);
  return {};
}

}

// Generate with moc -i from StarModUploader.cppm; compile in this scanned owner.
#include "moc_StarModUploader.cpp"
