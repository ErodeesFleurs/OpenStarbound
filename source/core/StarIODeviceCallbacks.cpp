#include "StarIODeviceCallbacks.hpp"
#include "vorbis/vorbisfile.h"

import std;

namespace Star {

IODeviceCallbacks::IODeviceCallbacks(Ptr<IODevice> device)
    : m_device(std::move(device)) {
  if (!m_device->isOpen())
    m_device->open(IOMode::Read);
}

auto IODeviceCallbacks::device() const -> Ptr<IODevice> const& {
  return m_device;
}

auto IODeviceCallbacks::readFunc(void* ptr, size_t size, size_t nmemb, void* datasource) -> size_t {
  auto* callbacks = static_cast<IODeviceCallbacks*>(datasource);
  return callbacks->m_device->read((char*)ptr, size * nmemb) / size;
}

auto IODeviceCallbacks::seekFunc(void* datasource, ogg_int64_t offset, int whence) -> int {
  auto* callbacks = static_cast<IODeviceCallbacks*>(datasource);
  callbacks->m_device->seek(offset, (IOSeek)whence);
  return 0;
}

auto IODeviceCallbacks::tellFunc(void* datasource) -> long int {
  auto* callbacks = static_cast<IODeviceCallbacks*>(datasource);
  return (long int)callbacks->m_device->pos();
}

void IODeviceCallbacks::setupOggCallbacks(ov_callbacks& callbacks) {
  callbacks.read_func = readFunc;
  callbacks.seek_func = seekFunc;
  callbacks.tell_func = tellFunc;
  callbacks.close_func = nullptr;
}

}// namespace Star
