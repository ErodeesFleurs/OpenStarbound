module;
#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QImage>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>

#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
import star.steam_dispatch;

import std;
import star.io_device;
import star.json;
import star.map;
import star.maybe;
import star.asset_source;
import star.string;
import star.directory_asset_source;
import star.s_plain_text_edit;

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

  Maybe<std::pair<CreateItemResult_t, bool>> m_steamItemCreateResult;
  Maybe<std::pair<SubmitItemUpdateResult_t, bool>> m_steamItemSubmitResult;
};

}


export module star.mod_uploader;

export namespace Star {
  using ::Star::ModUploader;
}
