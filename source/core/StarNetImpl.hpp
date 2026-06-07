#pragma once

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "StarString_windows.hpp"
#else
#ifdef STAR_SYSTEM_FREEBSD
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#endif

#include "StarHostAddress.hpp"

#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

namespace Star {

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
struct WindowsSocketInitializer {
  WindowsSocketInitializer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      fatalError("WSAStartup failed", false);
  };
};
static WindowsSocketInitializer g_windowsSocketInitializer;
#endif

// Platform trait that encapsulates all socket-level platform differences.
// Each platform specialization provides: SocketDesc, errorString(), connectionReset(),
// isInterrupt(), and setSockOpt().
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
struct SocketPlatform {
  using SocketDesc = SOCKET;
  static constexpr SOCKET invalidSocketDesc = INVALID_SOCKET;

  static bool isInvalid(SOCKET s) { return s == INVALID_SOCKET; }

  static String errorString() {
    LPWSTR lpMsgBuf = nullptr;
    int error = WSAGetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                  | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&lpMsgBuf),
        0,
        nullptr);

    String result = strf("{} - {}", error, utf16ToString(lpMsgBuf));

    if (lpMsgBuf != nullptr)
      LocalFree(lpMsgBuf);

    return result;
  }

  static bool connectionReset() {
    return WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAENETRESET;
  }

  static bool isInterrupt() {
    return WSAGetLastError() == WSAEINTR || WSAGetLastError() == WSAEWOULDBLOCK;
  }

  static void setSockOpt(SOCKET s, int level, int optname, const void* optval, socklen_t len) {
    int ret = ::setsockopt(s, level, optname, static_cast<const char*>(optval), len);
    if (ret < 0)
      throw NetworkException(strf("setSockOpt failed to set {}, {}: {}", level, optname, errorString()));
  }
};
#else
struct SocketPlatform {
  using SocketDesc = int;
  static constexpr int invalidSocketDesc = -1;

  static bool isInvalid(int s) { return s < 0; }

  static String errorString() {
    return strf("{} - {}", errno, strerror(errno));
  }

  static bool connectionReset() {
    return errno == ECONNRESET || errno == ETIMEDOUT;
  }

  static bool isInterrupt() {
    return errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK;
  }

  static void setSockOpt(int s, int level, int optname, const void* optval, socklen_t len) {
    int ret = ::setsockopt(s, level, optname, optval, len);
    if (ret < 0)
      throw NetworkException(strf("setSockOpt failed to set {}, {}: {}", level, optname, errorString()));
  }
};
#endif

inline String netErrorString() {
  return SocketPlatform::errorString();
}

inline bool netErrorConnectionReset() {
  return SocketPlatform::connectionReset();
}

inline bool netErrorInterrupt() {
  return SocketPlatform::isInterrupt();
}

inline void setAddressFromNative(HostAddressWithPort& addressWithPort, NetworkMode mode, struct sockaddr_storage* sockAddr) {
  switch (mode) {
    case NetworkMode::IPv4: {
      auto* addr4 = reinterpret_cast<struct sockaddr_in*>(sockAddr);
      addressWithPort = HostAddressWithPort(mode, reinterpret_cast<uint8_t*>(&addr4->sin_addr.s_addr), ntohs(addr4->sin_port));
      break;
    }
    case NetworkMode::IPv6: {
      auto* addr6 = reinterpret_cast<struct sockaddr_in6*>(sockAddr);
      addressWithPort = HostAddressWithPort(mode, reinterpret_cast<uint8_t*>(&addr6->sin6_addr.s6_addr), ntohs(addr6->sin6_port));
      break;
    }
    default:
      throw NetworkException("Invalid network mode for setAddressFromNative");
  }
}

inline void setNativeFromAddress(HostAddressWithPort const& addressWithPort, struct sockaddr_storage* sockAddr, socklen_t* sockAddrLen) {
  switch (addressWithPort.address().mode()) {
    case NetworkMode::IPv4: {
      auto* addr4 = reinterpret_cast<struct sockaddr_in*>(sockAddr);
      *sockAddrLen = sizeof(*addr4);

      memset(addr4, 0, *sockAddrLen);
      addr4->sin_family = AF_INET;
      addr4->sin_port = htons(addressWithPort.port());
      memcpy(reinterpret_cast<char*>(&addr4->sin_addr.s_addr), addressWithPort.address().bytes(), addressWithPort.address().size());

      break;
    }
    case NetworkMode::IPv6: {
      auto* addr6 = reinterpret_cast<struct sockaddr_in6*>(sockAddr);
      *sockAddrLen = sizeof(*addr6);

      memset(addr6, 0, *sockAddrLen);
      addr6->sin6_family = AF_INET6;
      addr6->sin6_port = htons(addressWithPort.port());
      memcpy(reinterpret_cast<char*>(&addr6->sin6_addr.s6_addr), addressWithPort.address().bytes(), addressWithPort.address().size());
      break;
    }
    default:
      throw NetworkException("Invalid network mode for setNativeFromAddress");
  }
}

inline bool invalidSocketDescriptor(typename SocketPlatform::SocketDesc socket) {
  return SocketPlatform::isInvalid(socket);
}

struct SocketImpl {
  SocketImpl() {
    socketDesc = 0;
  }

  void setSockOpt(int level, int optname, const void* optval, socklen_t len) {
    SocketPlatform::setSockOpt(socketDesc, level, optname, optval, len);
  }

  SocketPlatform::SocketDesc socketDesc;
};

}
