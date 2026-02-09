#include "StarConfig.hpp"
#include "StarLogging.hpp"
#include "StarRootLoader.hpp"
#include "StarServerQueryThread.hpp"
#include "StarServerRconThread.hpp"
#include "StarSignalHandler.hpp"
#include "StarUniverseServer.hpp"
#include "StarVersion.hpp"

import std;

using namespace Star;

#if defined STAR_SYSTEM_WINDOWS
#include <windows.h>
#endif

Json const AdditionalDefaultConfiguration = Json::parseJson(R"JSON(
    {
      "configurationVersion" : {
        "server" : 4
      },

      "runQueryServer" : false,
      "queryServerPort" : 21025,
      "queryServerBind" : "::",

      "runRconServer" : false,
      "rconServerPort" : 21026,
      "rconServerBind" : "::",
      "rconServerPassword" : "",
      "rconServerTimeout" : 1000,

      "allowAssetsMismatch" : true,
      "serverOverrideAssetsDigest" : null
    }
  )JSON");

auto main(int argc, char** argv) -> int {
  try {
#if defined STAR_SYSTEM_WINDOWS
    unsigned long exceptionStackSize = 131072;
    SetThreadStackGuarantee(&exceptionStackSize);
#endif
    RootLoader rootLoader({.additionalAssetsSettings = {}, .additionalDefaultConfiguration = AdditionalDefaultConfiguration, .logFile = String("starbound_server.log"), .logLevel = LogLevel::Info, .quiet = false, .runtimeConfigFile = String("starbound_server.config")});
    UPtr<Root> root = rootLoader.commandInitOrDie(argc, argv).first;
    root->fullyLoad();

    SignalHandler signalHandler;
    signalHandler.setHandleFatal(true);
    signalHandler.setHandleInterrupt(true);

    auto configuration = root->configuration();
    {
      Logger::info("OpenStarbound Server v{} for v{} ({}) Source ID: {} Protocol: {}", OpenStarVersionString, StarVersionString, StarArchitectureString, StarSourceIdentifierString, StarProtocolVersion);

      float updateRate = 1.0f / GlobalTimestep;
      if (auto jUpdateRate = configuration->get("updateRate")) {
        updateRate = jUpdateRate.toFloat();
        ServerGlobalTimestep = GlobalTimestep = 1.0f / updateRate;
        Logger::info("Configured tick rate is {:4.2f}hz", updateRate);
      }

      UPtr<UniverseServer> server = std::make_unique<UniverseServer>(root->toStoragePath("universe"));
      server->setListeningTcp(true);
      server->start();

      UPtr<ServerQueryThread> queryServer;
      if (configuration->get("runQueryServer").toBool()) {
        queryServer = std::make_unique<ServerQueryThread>(server.get(), HostAddressWithPort(configuration->get("queryServerBind").toString(), configuration->get("queryServerPort").toInt()));
        queryServer->start();
      }

      UPtr<ServerRconThread> rconServer;
      if (configuration->get("runRconServer").toBool()) {
        rconServer = std::make_unique<ServerRconThread>(server.get(), HostAddressWithPort(configuration->get("rconServerBind").toString(), configuration->get("rconServerPort").toInt()));
        rconServer->start();
      }

      while (server->isRunning()) {
        if (signalHandler.interruptCaught()) {
          Logger::info("Interrupt caught!");
          server->stop();
          break;
        }
        Thread::sleep(100);
      }

      server->join();

      if (queryServer) {
        queryServer->stop();
        queryServer->join();
      }

      if (rconServer) {
        rconServer->stop();
        rconServer->join();
      }
    }

    Logger::info("Server shutdown gracefully");
  } catch (std::exception const& e) {
    fatalException(e, true);
  }

  return 0;
}
