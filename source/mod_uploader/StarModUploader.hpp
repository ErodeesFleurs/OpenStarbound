#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>

#include "../application/StarSteamDispatch.hpp"

#include "StarIODevice.hpp"
#include "StarJson.hpp"
import star.asset_source;
#include "StarString.hpp"
import star.directory_asset_source;
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
  void closeEvent(QCloseEvent* event) override;
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

  SteamDispatch m_steamDispatch;
  bool m_closing = false;

  Maybe<pair<CreateItemResult_t, bool>> m_steamItemCreateResult;
  Maybe<pair<SubmitItemUpdateResult_t, bool>> m_steamItemSubmitResult;
};

}
