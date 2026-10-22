module;

import std;
import star.format;
import star.string;
import star.vector;
import star.input_event;
import star.logging;

namespace Star {

class ApplicationController;
class Renderer;

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
  [[nodiscard]] virtual std::expected<void, StarException> startup(StringList const& cmdLineArgs);

  // Available after startup and until this application is destroyed.
  virtual Logger& logger() const = 0;

  // Called on application initialization, before rendering initialization.  If
  // overriden, must call base class instance.
  [[nodiscard]] virtual std::expected<void, StarException> applicationInit(std::shared_ptr<ApplicationController> appController);

  // Called immediately after application initialization on startup, and then
  // also whenever the renderer invalidated and recreated.  If overridden, must
  // call base class instance.
  [[nodiscard]] virtual std::expected<void, StarException> renderInit(std::shared_ptr<Renderer> renderer);

  // Called when the window mode or size is changed.
  [[nodiscard]] virtual std::expected<void, StarException> windowChanged(WindowMode windowMode, Vec2U screenSize);

  // Called before update, once for every pending event.
  [[nodiscard]] virtual std::expected<void, StarException> processInput(InputEvent const& event);

  // Will be called at updateRate hz, or as close as possible.
  [[nodiscard]] virtual std::expected<void, StarException> update();
  
  // Returns how many frames have been skipped.
  virtual unsigned framesSkipped() const;

  // Will be called at updateRate hz, or more or less depending on settings and
  // performance.  update() is always prioritized over render().
  [[nodiscard]] virtual std::expected<void, StarException> render();

  // Will be called *from a different thread* to retrieve audio data (if audio
  // is playing). Default implementation simply fills the buffer with silence.
  [[nodiscard]] virtual std::expected<void, StarException> getAudioData(std::int16_t* sampleData, std::size_t frameCount);

  // Will be called once on application shutdown, including when shutting down
  // due to an Application exception.
  [[nodiscard]] virtual std::expected<void, StarException> shutdown();

  std::shared_ptr<ApplicationController> const& appController() const;
  std::shared_ptr<Renderer> const& renderer() const;

private:
  std::shared_ptr<ApplicationController> m_appController;
  std::shared_ptr<Renderer> m_renderer;
};

inline std::shared_ptr<ApplicationController> const& Application::appController() const {
  return m_appController;
}

inline std::shared_ptr<Renderer> const& Application::renderer() const {
  return m_renderer;
}

}

export module star.application;

export namespace Star {
  using ::Star::ApplicationController;
  using ::Star::Renderer;
  using ::Star::Application;
  using ::Star::ApplicationExceptionTag;
  using ::Star::ApplicationException;
  using ::Star::WindowMode;
}

