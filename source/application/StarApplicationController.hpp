#pragma once

#include "StarApplication.hpp"
#include "StarConfig.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarStatisticsService.hpp"
#include "StarUserGeneratedContentService.hpp"

import std;

namespace Star {
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
  virtual void setVSyncEnabled(bool vSync) = 0;
  virtual void setCursorVisible(bool cursorVisible) = 0;
  virtual void setCursorPosition(Vec2I cursorPosition) = 0;
  virtual void setCursorHardware(bool cursorHardware) = 0;
  virtual auto setCursorImage(const String& id, const ConstPtr<Image>& image, unsigned scale, const Vec2I& offset) -> bool = 0;
  virtual void setAcceptingTextInput(bool acceptingTextInput) = 0;
  virtual void setTextArea(std::optional<std::pair<RectI, int>> area = {}) = 0;

  virtual auto enableAudio() -> AudioFormat = 0;
  virtual void disableAudio() = 0;

  using AudioCallback = std::function<void(std::uint8_t*, int)>;
  virtual auto openAudioInputDevice(std::uint32_t deviceId, int freq, int channels, AudioCallback callback) -> bool = 0;
  virtual auto closeAudioInputDevice() -> bool = 0;

  virtual auto hasClipboard() -> bool = 0;
  virtual auto setClipboard(String text) -> bool = 0;
  virtual auto setClipboardData(StringMap<ByteArray>) -> bool = 0;
  virtual auto setClipboardImage(Image const& image, ByteArray* png = {}, String const* path = nullptr) -> bool = 0;
  virtual auto setClipboardFile(String const& path) -> bool = 0;
  virtual auto getClipboard() -> std::optional<String> = 0;

  [[nodiscard]] virtual auto isFocused() const -> bool = 0;

  // Returns the latest actual measured update and render rate, which may be
  // different than the target update rate.
  [[nodiscard]] virtual auto updateRate() const -> float = 0;
  [[nodiscard]] virtual auto renderFps() const -> float = 0;

  [[nodiscard]] virtual auto statisticsService() const -> Ptr<StatisticsService> = 0;
  [[nodiscard]] virtual auto p2pNetworkingService() const -> Ptr<P2PNetworkingService> = 0;
  [[nodiscard]] virtual auto userGeneratedContentService() const -> Ptr<UserGeneratedContentService> = 0;
  [[nodiscard]] virtual auto desktopService() const -> Ptr<DesktopService> = 0;

  // Signals the application to quit
  virtual void quit() = 0;
};

}// namespace Star
