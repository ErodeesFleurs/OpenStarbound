module;
import std;
import star.byte_array;

import star.maybe;
import star.string;
import star.input_event;
import star.application;
import star.map;
import star.rect;

import star.vector;
namespace Star {

class ApplicationController;
using ApplicationControllerPtr = std::shared_ptr<ApplicationController>;
using ApplicationControllerConstPtr = std::shared_ptr<ApplicationController const>;
using ApplicationControllerWeakPtr = std::weak_ptr<ApplicationController>;
using ApplicationControllerConstWeakPtr = std::weak_ptr<ApplicationController const>;
using ApplicationControllerUPtr = std::unique_ptr<ApplicationController>;
using ApplicationControllerConstUPtr = std::unique_ptr<ApplicationController const>;
class StatisticsService;
using StatisticsServicePtr = std::shared_ptr<StatisticsService>;
using StatisticsServiceConstPtr = std::shared_ptr<StatisticsService const>;
using StatisticsServiceWeakPtr = std::weak_ptr<StatisticsService>;
using StatisticsServiceConstWeakPtr = std::weak_ptr<StatisticsService const>;
using StatisticsServiceUPtr = std::unique_ptr<StatisticsService>;
using StatisticsServiceConstUPtr = std::unique_ptr<StatisticsService const>;
class P2PNetworkingService;
using P2PNetworkingServicePtr = std::shared_ptr<P2PNetworkingService>;
using P2PNetworkingServiceConstPtr = std::shared_ptr<P2PNetworkingService const>;
using P2PNetworkingServiceWeakPtr = std::weak_ptr<P2PNetworkingService>;
using P2PNetworkingServiceConstWeakPtr = std::weak_ptr<P2PNetworkingService const>;
using P2PNetworkingServiceUPtr = std::unique_ptr<P2PNetworkingService>;
using P2PNetworkingServiceConstUPtr = std::unique_ptr<P2PNetworkingService const>;
class UserGeneratedContentService;
using UserGeneratedContentServicePtr = std::shared_ptr<UserGeneratedContentService>;
using UserGeneratedContentServiceConstPtr = std::shared_ptr<UserGeneratedContentService const>;
using UserGeneratedContentServiceWeakPtr = std::weak_ptr<UserGeneratedContentService>;
using UserGeneratedContentServiceConstWeakPtr = std::weak_ptr<UserGeneratedContentService const>;
using UserGeneratedContentServiceUPtr = std::unique_ptr<UserGeneratedContentService>;
using UserGeneratedContentServiceConstUPtr = std::unique_ptr<UserGeneratedContentService const>;
class DesktopService;
using DesktopServicePtr = std::shared_ptr<DesktopService>;
using DesktopServiceConstPtr = std::shared_ptr<DesktopService const>;
using DesktopServiceWeakPtr = std::weak_ptr<DesktopService>;
using DesktopServiceConstWeakPtr = std::weak_ptr<DesktopService const>;
using DesktopServiceUPtr = std::unique_ptr<DesktopService>;
using DesktopServiceConstUPtr = std::unique_ptr<DesktopService const>;
class Image;
using ImagePtr = std::shared_ptr<Image>;
using ImageConstPtr = std::shared_ptr<Image const>;
using ImageWeakPtr = std::weak_ptr<Image>;
using ImageConstWeakPtr = std::weak_ptr<Image const>;
using ImageUPtr = std::unique_ptr<Image>;
using ImageConstUPtr = std::unique_ptr<Image const>;

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
  virtual void setFullscreenWindow(Vec2U fullScreenResolution) = 0;
  virtual void setNormalWindow(Vec2U windowSize) = 0;
  virtual void setMaximizedWindow() = 0;
  virtual void setBorderlessWindow() = 0;
  #ifdef STAR_SYSTEM_WINDOWS
  virtual void setBorderlessWorkaround(bool active) = 0;
  #endif
  virtual void setVSyncEnabled(bool vSync) = 0;
  virtual void setCursorVisible(bool cursorVisible) = 0;
  virtual void setCursorPosition(Vec2I cursorPosition) = 0;
  virtual void setCursorHardware(bool cursorHardware) = 0;
  virtual bool setCursorImage(const String& id, const ImageConstPtr& image, unsigned scale, const Vec2I& offset) = 0;
  virtual void setAcceptingTextInput(bool acceptingTextInput) = 0;
  virtual void setTextArea(Maybe<std::pair<RectI, int>> area = {}) = 0;


  virtual AudioFormat enableAudio() = 0;
  virtual void disableAudio() = 0;

  typedef std::function<void(std::uint8_t*, int)> AudioCallback;
  virtual bool openAudioInputDevice(std::uint32_t deviceId, int freq, int channels, AudioCallback callback) = 0;
  virtual bool closeAudioInputDevice() = 0;

  virtual bool hasClipboard() = 0;
  virtual bool setClipboard(String text) = 0;
  virtual bool setClipboardData(StringMap<ByteArray>) = 0;
  virtual bool setClipboardImage(Image const& image, ByteArray* png = {}, String const* path = nullptr) = 0;
  virtual bool setClipboardFile(String const& path) = 0;
  virtual Maybe<String> getClipboard() = 0;

  virtual bool isFocused() const = 0;

  // Returns the latest actual measured update and render rate, which may be
  // different than the target update rate.
  virtual float updateRate() const = 0;
  virtual float renderFps() const = 0;

  virtual float getDisplayScale() const = 0;

  virtual StatisticsServicePtr statisticsService() const = 0;
  virtual P2PNetworkingServicePtr p2pNetworkingService() const = 0;
  virtual UserGeneratedContentServicePtr userGeneratedContentService() const = 0;
  virtual DesktopServicePtr desktopService() const = 0;

  // Signals the application to quit
  virtual void quit() = 0;
};

}

export module star.application_controller;

export namespace Star {
  using ::Star::ApplicationController;
  using ::Star::ApplicationControllerPtr;
  using ::Star::ApplicationControllerConstPtr;
  using ::Star::ApplicationControllerUPtr;
  using ::Star::ApplicationControllerConstUPtr;
  using ::Star::ApplicationControllerWeakPtr;
  using ::Star::ApplicationControllerConstWeakPtr;
  using ::Star::AudioFormat;
}
