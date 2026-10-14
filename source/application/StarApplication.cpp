import std;
import star.vector;
import star.input_event;
import star.application;
import star.thread;
import star.time;
import star.logging;
import star.string;

namespace Star {

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#ifdef STAR_SYSTEM_LINUX
// Hacky, but I need to use this from StarClientApplication
// and StarPlatformServies_pc...
bool g_steamIsFlatpak = false;
#endif
#endif

void Application::startup(StringList const&) {}

void Application::applicationInit(ApplicationControllerPtr appController) {
  m_appController = std::move(appController);
}

void Application::renderInit(RendererPtr renderer) {
  m_renderer = std::move(renderer);
}

void Application::windowChanged(WindowMode, Vec2U) {}

void Application::processInput(InputEvent const&) {}

void Application::update() {}

unsigned Application::framesSkipped() const { return 0; }

void Application::render() {}

void Application::getAudioData(std::int16_t* samples, std::size_t sampleCount) {
  for (std::size_t i = 0; i < sampleCount; ++i)
    samples[i] = 0;
}

void Application::shutdown() {}
}
