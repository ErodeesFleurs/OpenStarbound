#pragma once

#include "StarInputEvent.hpp"

namespace Star {

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#ifdef STAR_SYSTEM_LINUX
extern bool g_steamIsFlatpak;
#endif
#endif

class ApplicationController;
using ApplicationControllerPtr = SharedPtr<ApplicationController>;
using ApplicationControllerConstPtr = SharedPtr<ApplicationController const>;
using ApplicationControllerWeakPtr = WeakPtr<ApplicationController>;
using ApplicationControllerConstWeakPtr = WeakPtr<ApplicationController const>;
using ApplicationControllerUPtr = UniquePtr<ApplicationController>;
using ApplicationControllerConstUPtr = UniquePtr<ApplicationController const>;
class Renderer;
using RendererPtr = SharedPtr<Renderer>;
using RendererConstPtr = SharedPtr<Renderer const>;
using RendererWeakPtr = WeakPtr<Renderer>;
using RendererConstWeakPtr = WeakPtr<Renderer const>;
using RendererUPtr = UniquePtr<Renderer>;
using RendererConstUPtr = UniquePtr<Renderer const>;
class Application;
using ApplicationPtr = SharedPtr<Application>;
using ApplicationConstPtr = SharedPtr<Application const>;
using ApplicationWeakPtr = WeakPtr<Application>;
using ApplicationConstWeakPtr = WeakPtr<Application const>;
using ApplicationUPtr = UniquePtr<Application>;
using ApplicationConstUPtr = UniquePtr<Application const>;

struct ApplicationExceptionTag { static constexpr char const* typeName = "ApplicationException"; };
using ApplicationException = TypedException<StarException, ApplicationExceptionTag>;

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
  virtual void applicationInit(ApplicationControllerPtr appController);

  // Called immediately after application initialization on startup, and then
  // also whenever the renderer invalidated and recreated.  If overridden, must
  // call base class instance.
  virtual void renderInit(RendererPtr renderer);

  // Called when the window mode or size is changed.
  virtual void windowChanged(WindowMode windowMode, Vec2U screenSize);

  // Called before update, once for every pending event.
  virtual void processInput(InputEvent const& event);

  // Will be called at updateRate hz, or as close as possible.
  virtual void update();
  
  // Returns how many frames have been skipped.
  virtual unsigned framesSkipped() const;

  // Will be called at updateRate hz, or more or less depending on settings and
  // performance.  update() is always prioritized over render().
  virtual void render();

  // Will be called *from a different thread* to retrieve audio data (if audio
  // is playing). Default implementation simply fills the buffer with silence.
  virtual void getAudioData(int16_t* sampleData, size_t frameCount);

  // Will be called once on application shutdown, including when shutting down
  // due to an Application exception.
  virtual void shutdown();

  ApplicationControllerPtr const& appController() const;
  RendererPtr const& renderer() const;

private:
  ApplicationControllerPtr m_appController;
  RendererPtr m_renderer;
};

inline ApplicationControllerPtr const& Application::appController() const {
  return m_appController;
}

inline RendererPtr const& Application::renderer() const {
  return m_renderer;
}

}
