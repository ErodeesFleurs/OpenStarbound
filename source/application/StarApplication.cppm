module;

import std;
import star.format;
import star.string;
import star.vector;
import star.input_event;

namespace Star {

#ifdef STAR_ENABLE_STEAM_INTEGRATION
#ifdef STAR_SYSTEM_LINUX
extern bool g_steamIsFlatpak;
#endif
#endif

class ApplicationController;
using ApplicationControllerPtr = std::shared_ptr<ApplicationController>;
using ApplicationControllerConstPtr = std::shared_ptr<ApplicationController const>;
using ApplicationControllerWeakPtr = std::weak_ptr<ApplicationController>;
using ApplicationControllerConstWeakPtr = std::weak_ptr<ApplicationController const>;
using ApplicationControllerUPtr = std::unique_ptr<ApplicationController>;
using ApplicationControllerConstUPtr = std::unique_ptr<ApplicationController const>;
class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;
using RendererConstPtr = std::shared_ptr<Renderer const>;
using RendererWeakPtr = std::weak_ptr<Renderer>;
using RendererConstWeakPtr = std::weak_ptr<Renderer const>;
using RendererUPtr = std::unique_ptr<Renderer>;
using RendererConstUPtr = std::unique_ptr<Renderer const>;
class Application;
using ApplicationPtr = std::shared_ptr<Application>;
using ApplicationConstPtr = std::shared_ptr<Application const>;
using ApplicationWeakPtr = std::weak_ptr<Application>;
using ApplicationConstWeakPtr = std::weak_ptr<Application const>;
using ApplicationUPtr = std::unique_ptr<Application>;
using ApplicationConstUPtr = std::unique_ptr<Application const>;

struct ApplicationExceptionTag {
  static constexpr char const* name() { return "ApplicationException"; }
};
using ApplicationException = StarError<ApplicationExceptionTag, StarException>;

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
  virtual void getAudioData(std::int16_t* sampleData, std::size_t frameCount);

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

export module star.application;

export namespace Star {
  using ::Star::ApplicationController;
  using ::Star::ApplicationControllerPtr;
  using ::Star::ApplicationControllerConstPtr;
  using ::Star::ApplicationControllerUPtr;
  using ::Star::ApplicationControllerConstUPtr;
  using ::Star::ApplicationControllerWeakPtr;
  using ::Star::ApplicationControllerConstWeakPtr;
  using ::Star::Renderer;
  using ::Star::RendererPtr;
  using ::Star::RendererConstPtr;
  using ::Star::RendererUPtr;
  using ::Star::RendererConstUPtr;
  using ::Star::RendererWeakPtr;
  using ::Star::RendererConstWeakPtr;
  using ::Star::Application;
  using ::Star::ApplicationPtr;
  using ::Star::ApplicationConstPtr;
  using ::Star::ApplicationUPtr;
  using ::Star::ApplicationConstUPtr;
  using ::Star::ApplicationWeakPtr;
  using ::Star::ApplicationConstWeakPtr;
  using ::Star::ApplicationExceptionTag;
  using ::Star::ApplicationException;
  using ::Star::WindowMode;
}

#if defined(STAR_ENABLE_STEAM_INTEGRATION) && defined(STAR_SYSTEM_LINUX)
export namespace Star {
  using ::Star::g_steamIsFlatpak;
}
#endif
