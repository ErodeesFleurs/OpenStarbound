#include "StarApplication.hpp"

#include "StarConfig.hpp"

import std;

namespace Star {

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#ifdef STAR_SYSTEM_LINUX
// Hacky, but I need to use this from StarClientApplication
// and StarPlatformServies_pc...
bool g_steamIsFlatpak = false;
#endif
#endif

void Application::startup(StringList const&) {}

void Application::applicationInit(Ptr<ApplicationController> appController) {
  m_appController = std::move(appController);
}

void Application::renderInit(Ptr<Renderer> renderer) {
  m_renderer = std::move(renderer);
}

void Application::windowChanged(WindowMode, Vec2U) {}

void Application::processInput(InputEvent const&) {}

void Application::update() {}

auto Application::framesSkipped() const -> unsigned { return 0; }

void Application::render() {}

void Application::getAudioData(std::int16_t* samples, std::size_t sampleCount) {
  for (std::size_t i = 0; i < sampleCount; ++i)
    samples[i] = 0;
}

void Application::shutdown() {}
}// namespace Star
