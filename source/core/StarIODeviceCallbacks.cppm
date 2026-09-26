module;

#include "StarIODevice.hpp"
#define OV_EXCLUDE_STATIC_CALLBACKS
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

export module star.io_device_callbacks;

export namespace Star {

// Provides callbacks for interfacing IODevice with ogg vorbis callbacks
class IODeviceCallbacks {
public:
  explicit IODeviceCallbacks(IODevicePtr device);

  // No copying
  IODeviceCallbacks(IODeviceCallbacks const&) = delete;
  IODeviceCallbacks& operator=(IODeviceCallbacks const&) = delete;

  // Moving is ok
  IODeviceCallbacks(IODeviceCallbacks&&) = default;
  IODeviceCallbacks& operator=(IODeviceCallbacks&&) = default;

  // Get the underlying device
  IODevicePtr const& device() const;

  // Callback functions for Ogg Vorbis
  static size_t readFunc(void* ptr, size_t size, size_t nmemb, void* datasource);
  static int seekFunc(void* datasource, ogg_int64_t offset, int whence);
  static long int tellFunc(void* datasource);

  // Sets up callbacks for Ogg Vorbis
  void setupOggCallbacks(ov_callbacks& callbacks);

private:
  IODevicePtr m_device;
};

}

namespace Star {

IODeviceCallbacks::IODeviceCallbacks(IODevicePtr device)
  : m_device(std::move(device)) {
  if (!m_device->isOpen())
    m_device->open(IOMode::Read);
}

IODevicePtr const& IODeviceCallbacks::device() const {
  return m_device;
}

size_t IODeviceCallbacks::readFunc(void* ptr, size_t size, size_t nmemb, void* datasource) {
  auto* callbacks = static_cast<IODeviceCallbacks*>(datasource);
  return callbacks->m_device->read((char*)ptr, size * nmemb) / size;
}

int IODeviceCallbacks::seekFunc(void* datasource, ogg_int64_t offset, int whence) {
  auto* callbacks = static_cast<IODeviceCallbacks*>(datasource);
  callbacks->m_device->seek(offset, (IOSeek)whence);
  return 0;
}

long int IODeviceCallbacks::tellFunc(void* datasource) {
  auto* callbacks = static_cast<IODeviceCallbacks*>(datasource);
  return (long int)callbacks->m_device->pos();
}

void IODeviceCallbacks::setupOggCallbacks(ov_callbacks& callbacks) {
  callbacks.read_func = readFunc;
  callbacks.seek_func = seekFunc;
  callbacks.tell_func = tellFunc;
  callbacks.close_func = nullptr;
}

}
