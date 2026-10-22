module;
#include <QErrorMessage>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>

import star.json_tool;
import std;
import star.string;
import star.list;
import star.formatted_json;
import star.json_path;
import star.logging;
import star.format;

namespace Star {

class JsonEditor : public QMainWindow {
  Q_OBJECT

public:
  explicit JsonEditor(std::shared_ptr<JsonPath::Path> const& path, Options const& options, List<String> const& files);

private slots:
  void next();
  void back();
  void updatePreview(QString const& valueStr);

private:
  // Returns false if the change can't be made or the edit is invalid Json
  bool saveChanges();

  void displayCurrentFile();
  void updateValueEditor();
  void updateImagePreview();

  QLabel* m_statusLabel;
  QLabel* m_pathLabel;
  QLabel* m_imageLabel;
  QTextEdit* m_jsonPreview;
  QTextDocument* m_jsonDocument;
  QLineEdit* m_valueEditor;
  QErrorMessage* m_errorDialog;
  QPushButton* m_backButton;
  QPushButton* m_nextButton;

  std::shared_ptr<JsonPath::Path> m_path;
  std::shared_ptr<JsonInputFormat> m_editFormat;
  Options m_options;
  List<String> m_files;
  std::size_t m_fileIndex;
  FormattedJson m_currentJson;
};

[[nodiscard]] std::expected<int, StarException> edit(Logger& logger, int argc, char* argv[], std::shared_ptr<JsonPath::Path> const& path, Options const& options, List<Input> const& inputs);

}


export module star.json_editor;

export namespace Star {
  using ::Star::JsonEditor;
  using ::Star::edit;
}
