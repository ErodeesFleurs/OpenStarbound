#pragma once

#include "StarConfig.hpp"
#include "StarIODevice.hpp"
#include "vorbis/vorbisfile.h"

namespace Star {

// Provides callbacks for interfacing IODevice with ogg vorbis callbacks
class IODeviceCallbacks {
public:
  explicit IODeviceCallbacks(Ptr<IODevice> device);

  // No copying
  IODeviceCallbacks(IODeviceCallbacks const&) = delete;
  auto operator=(IODeviceCallbacks const&) -> IODeviceCallbacks& = delete;

  // Moving is ok
  IODeviceCallbacks(IODeviceCallbacks&&) = default;
  auto operator=(IODeviceCallbacks&&) -> IODeviceCallbacks& = default;

  // Get the underlying device
  [[nodiscard]] auto device() const -> Ptr<IODevice> const&;

  // Callback functions for Ogg Vorbis
  static auto readFunc(void* ptr, size_t size, size_t nmemb, void* datasource) -> size_t;
  static auto seekFunc(void* datasource, ogg_int64_t offset, int whence) -> int;
  static auto tellFunc(void* datasource) -> long int;

  // Sets up callbacks for Ogg Vorbis
  void setupOggCallbacks(ov_callbacks& callbacks);

private:
  Ptr<IODevice> m_device;
};

}
