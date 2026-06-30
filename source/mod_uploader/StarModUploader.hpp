#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>

#include "steam/steam_api.h"

#include "StarDirectoryAssetSource.hpp"
#include "StarSPlainTextEdit.hpp"

namespace Star {

class ModUploader : public QMainWindow {
  Q_OBJECT
public:
  ModUploader();

private slots:
  void selectDirectory();
  void loadDirectory();
  void selectPreview();
  void writeMetadata();
  void writePreview();
  void resetModId();
  void uploadToSteam();

private:
  void onSteamCreateItem(CreateItemResult_t* result, bool ioFailure);
  void onSteamSubmitItem(SubmitItemUpdateResult_t* result, bool ioFailure);

  QPushButton* m_reloadButton;
  QLabel* m_directoryLabel;
  QLineEdit* m_nameEditor;
  QLineEdit* m_titleEditor;
  QLineEdit* m_authorEditor;
  QLineEdit* m_versionEditor;
  SPlainTextEdit* m_descriptionEditor;
  QLabel* m_previewImageLabel;
  QLabel* m_modIdLabel;
  QWidget* m_editorSection;
  HashMap<String, QCheckBox*> m_categorySelectors;

  Maybe<String> m_modDirectory;
  Maybe<DirectoryAssetSource> m_assetSource;
  QImage m_modPreview;

  struct SteamItemCreateResult {
    CreateItemResult_t result = {};
    bool ioFailure = false;
  };

  struct SteamItemSubmitResult {
    SubmitItemUpdateResult_t result = {};
    bool ioFailure = false;
  };

  Maybe<SteamItemCreateResult> m_steamItemCreateResult;
  Maybe<SteamItemSubmitResult> m_steamItemSubmitResult;
};

}
