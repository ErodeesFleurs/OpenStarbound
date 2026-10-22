module;
import std;
import star.format;
import star.byte_array;

import star.string;
import star.input_event;
import star.application;
import star.map;
import star.rect;

import star.vector;
namespace Star {

class StatisticsService;
class P2PNetworkingService;
class UserGeneratedContentService;
class DesktopService;
class Image;

// Audio format is always 16 bit signed integer samples
struct AudioFormat {
  unsigned sampleRate;
  unsigned channels;
};

// Window size defaults to 800x600, target update rate to 60hz, maximized and
// fullscreen are false, vsync is on, the cursor is visible, and audio and text
// input are disabled.
class ApplicationController {
public:
  virtual ~ApplicationController() = default;

  // Target hz at which update() will be called
  virtual void setTargetUpdateRate(float targetUpdateRate) = 0;
  // Window that controls how long the update rate will be increased or
  // decreased to make up for rate errors in the past.
  virtual void setUpdateTrackWindow(float updateTrackWindow) = 0;
  // Maximum number of calls to update() that can occur before we force
  // 'render()' to be called, even if we are still behind on our update rate.
  virtual void setMaxFrameSkip(unsigned maxFrameSkip) = 0;

  virtual void setApplicationTitle(String title) = 0;
  [[nodiscard]] virtual std::expected<void, StarException> setFullscreenWindow(Vec2U fullScreenResolution) = 0;
  virtual void setNormalWindow(Vec2U windowSize) = 0;
  virtual void setMaximizedWindow() = 0;
  [[nodiscard]] virtual std::expected<void, StarException> setBorderlessWindow() = 0;
  #ifdef STAR_SYSTEM_WINDOWS
  virtual void setBorderlessWorkaround(bool active) = 0;
  #endif
  [[nodiscard]] virtual std::expected<void, StarException> setVSyncEnabled(bool vSync) = 0;
  virtual void setCursorVisible(bool cursorVisible) = 0;
  virtual void setCursorPosition(Vec2I cursorPosition) = 0;
  virtual void setCursorHardware(bool cursorHardware) = 0;
  [[nodiscard]] virtual std::expected<bool, StarException> setCursorImage(const String& id, const std::shared_ptr<Image const>& image, unsigned scale, const Vec2I& offset) = 0;
  virtual void setAcceptingTextInput(bool acceptingTextInput) = 0;
  virtual void setTextArea(std::optional<std::pair<RectI, int>> area = {}) = 0;


  virtual AudioFormat enableAudio() = 0;
  virtual void disableAudio() = 0;

  typedef std::function<void(std::uint8_t*, int)> AudioCallback;
  [[nodiscard]] virtual std::expected<bool, StarException> openAudioInputDevice(std::uint32_t deviceId, int freq, int channels, AudioCallback callback) = 0;
  virtual bool closeAudioInputDevice() = 0;

  virtual bool hasClipboard() = 0;
  virtual bool setClipboard(String text) = 0;
  virtual bool setClipboardData(StringMap<ByteArray>) = 0;
  [[nodiscard]] virtual std::expected<bool, StarException> setClipboardImage(Image const& image, ByteArray* png = {}, String const* path = nullptr) = 0;
  [[nodiscard]] virtual std::expected<bool, StarException> setClipboardFile(String const& path) = 0;
  virtual std::optional<String> getClipboard() = 0;

  virtual bool isFocused() const = 0;

  // Returns the latest actual measured update and render rate, which may be
  // different than the target update rate.
  virtual float updateRate() const = 0;
  virtual float renderFps() const = 0;

  virtual float getDisplayScale() const = 0;

  virtual std::shared_ptr<StatisticsService> statisticsService() const = 0;
  virtual std::shared_ptr<P2PNetworkingService> p2pNetworkingService() const = 0;
  virtual std::shared_ptr<UserGeneratedContentService> userGeneratedContentService() const = 0;
  virtual std::shared_ptr<DesktopService> desktopService() const = 0;
  virtual bool steamIsFlatpak() const = 0;

  // Signals the application to quit
  virtual void quit() = 0;
};

}

export module star.application_controller;

export namespace Star {
  using ::Star::ApplicationController;
  using ::Star::AudioFormat;
}
