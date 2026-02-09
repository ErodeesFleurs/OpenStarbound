#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarInputEvent.hpp"

import std;

namespace Star {

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#ifdef STAR_SYSTEM_LINUX
extern bool g_steamIsFlatpak;
#endif
#endif

class ApplicationController;
class Renderer;

using ApplicationException = ExceptionDerived<"ApplicationException">;

enum class WindowMode {
  Normal,
  Maximized,
  Fullscreen,
  Borderless
};

class Application {
public:
  virtual ~Application() = default;

  // Called once on application startup, before any other methods.
  virtual void startup(StringList const& cmdLineArgs);

  // Called on application initialization, before rendering initialization.  If
  // overriden, must call base class instance.
  virtual void applicationInit(Ptr<ApplicationController> appController);

  // Called immediately after application initialization on startup, and then
  // also whenever the renderer invalidated and recreated.  If overridden, must
  // call base class instance.
  virtual void renderInit(Ptr<Renderer> renderer);

  // Called when the window mode or size is changed.
  virtual void windowChanged(WindowMode windowMode, Vec2U screenSize);

  // Called before update, once for every pending event.
  virtual void processInput(InputEvent const& event);

  // Will be called at updateRate hz, or as close as possible.
  virtual void update();

  // Returns how many frames have been skipped.
  [[nodiscard]] virtual auto framesSkipped() const -> unsigned;

  // Will be called at updateRate hz, or more or less depending on settings and
  // performance.  update() is always prioritized over render().
  virtual void render();

  // Will be called *from a different thread* to retrieve audio data (if audio
  // is playing). Default implementation simply fills the buffer with silence.
  virtual void getAudioData(std::int16_t* sampleData, std::size_t frameCount);

  // Will be called once on application shutdown, including when shutting down
  // due to an Application exception.
  virtual void shutdown();

  [[nodiscard]] auto appController() const -> Ptr<ApplicationController> const&;
  [[nodiscard]] auto renderer() const -> Ptr<Renderer> const&;

private:
  Ptr<ApplicationController> m_appController;
  Ptr<Renderer> m_renderer;
};

inline auto Application::appController() const -> Ptr<ApplicationController> const& {
  return m_appController;
}

inline auto Application::renderer() const -> Ptr<Renderer> const& {
  return m_renderer;
}

}// namespace Star
