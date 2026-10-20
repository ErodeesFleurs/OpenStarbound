#include "steam/steam_api.h"
import std;
import star.format;

#include <QApplication>
#include <QMessageBox>


import star.mod_uploader;
import star.string_conversion;
import star.logging;
import star.random;
import star.native_runtime;

using namespace Star;

int main(int argc, char** argv) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  QApplication app(argc, argv);

  if (!SteamAPI_Init()) {
    QMessageBox::critical(nullptr, "Error", "Could not initialize Steam API");
    return 1;
  }
  struct SteamShutdown {
    ~SteamShutdown() { SteamAPI_Shutdown(); }
  } steamShutdown;

  Logger logger;
  RandomService random;
  ModUploader modUploader(logger, random);
  modUploader.show();

  try {
    return app.exec();
  } catch (std::exception const& e) {
    QMessageBox::critical(nullptr, "Error", toQString(strf("Exception caught: {}\n", outputException(e, true))));
    coutf("Exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
