module;
#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QImage>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>

class QProgressDialog;
#include "steam/steam_api.h"
#include "steam/steam_api_flat.h"
import star.steam_dispatch;

import std;
import star.io_device;
import star.json;
import star.map;
import star.asset_source;
import star.string;
import star.directory_asset_source;
import star.s_plain_text_edit;
import star.logging;
import star.random;
import star.format;

namespace Star {

// The entry point owns these services until the uploader and its callbacks stop.
class ModUploader : public QMainWindow {
  Q_OBJECT
public:
  [[nodiscard]] static std::expected<std::unique_ptr<ModUploader>, StarException> create(Logger& logger, RandomService& random);

private slots:
  void selectDirectory();
  void loadDirectory();
  void selectPreview();
  void writeMetadata();
  void resetModId();
  void uploadToSteam();

private:
  ModUploader(Logger& logger, RandomService& random);
  void showError(StarException const& error);
  [[nodiscard]] std::expected<void, StarException> loadDirectoryImpl();
  [[nodiscard]] std::expected<void, StarException> writeMetadataImpl();
  [[nodiscard]] std::expected<void, StarException> savePreview(QImage const& preview);
  [[nodiscard]] std::expected<void, StarException> resetModIdImpl();
  [[nodiscard]] std::expected<void, StarException> uploadToSteamImpl(QProgressDialog& progress);
  void closeEvent(QCloseEvent* event) override;
  [[nodiscard]] std::expected<void, StarException> onSteamCreateItem(CreateItemResult_t* result, bool ioFailure);
  [[nodiscard]] std::expected<void, StarException> onSteamSubmitItem(SubmitItemUpdateResult_t* result, bool ioFailure);

  Logger& m_logger;
  RandomService& m_random;

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

  std::optional<String> m_modDirectory;
  std::shared_ptr<DirectoryAssetSource> m_assetSource;
  QImage m_modPreview;

  SteamDispatch m_steamDispatch;
  bool m_closing = false;

  std::optional<std::pair<CreateItemResult_t, bool>> m_steamItemCreateResult;
  std::optional<std::pair<SubmitItemUpdateResult_t, bool>> m_steamItemSubmitResult;
};

}


export module star.mod_uploader;

export namespace Star {
  using ::Star::ModUploader;
}
