#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
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

import std;
import star.maybe;
import star.format;
import star.string;
import star.either;
import star.host_address;
import star.thread;
import star.socket;
import star.logging;
import star.net_impl;
import star.list;

namespace Star {

Maybe<SocketPollResult> Socket::poll(SocketPollQuery const& query, unsigned timeout) {
  if (query.empty())
    return {};

  // Prevent close from being called on any socket during this call.
  LinkedList<ReadLocker> readLockers;
  for (auto const& p : query)
    readLockers.emplaceAppend(p.first->m_mutex);

  // If any sockets are already closed, then this is an "event" according to
  // this api but we cannot call poll on a closed socket, so just poll the rest
  // of the sockets with no wait.
  SocketPollResult result;
  for (auto const& p : query) {
    if (!p.first->isOpen()) {
      result[p.first].exception = true;
      timeout = 0;
    }
  }

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  fd_set readfs;
  fd_set writefs;
  fd_set exceptfs;

  FD_ZERO(&readfs);
  FD_ZERO(&writefs);
  FD_ZERO(&exceptfs);

  int ret;
  for (auto const& p : query) {
    if (p.first->isOpen()) {
      if (p.second.readable)
        FD_SET(p.first->m_impl->socketDesc, &readfs);
      if (p.second.writable)
        FD_SET(p.first->m_impl->socketDesc, &writefs);
      FD_SET(p.first->m_impl->socketDesc, &exceptfs);
    }
  }
  timeval time;
  time.tv_usec = (timeout % 1000) * 1000;
  time.tv_sec = timeout - timeout % 1000;
  ret = ::select(0, &readfs, &writefs, &exceptfs, &time);

  if (ret < 0)
    throw NetworkException::format("Error during call to select, '{}'", netErrorString());

  if (ret == 0)
    return {};

  for (auto const& p : query) {
    if (p.first->isOpen()) {
      auto& r = result[p.first];
      r.readable = FD_ISSET(p.first->m_impl->socketDesc, &readfs);
      r.writable = FD_ISSET(p.first->m_impl->socketDesc, &writefs);
      r.exception = FD_ISSET(p.first->m_impl->socketDesc, &exceptfs);
      if (r.exception)
        p.first->doShutdown();
    }
  }

#else
  std::unique_ptr<pollfd[]> pollfds(new pollfd[query.size()]);
  int ret = 0;
  for (auto [index, p] : std::views::enumerate(query)) {
    if (p.first->isOpen()) {
      auto& pfd = pollfds[static_cast<std::size_t>(index)];
      pfd.fd = p.first->m_impl->socketDesc;
      pfd.events = 0;
      if (p.second.readable)
        pfd.events |= POLLIN;
      if (p.second.writable)
        pfd.events |= POLLOUT;
    }
  }
  ret = ::poll(pollfds.get(), query.size(), timeout);

  if (ret < 0)
    throw NetworkException::format("Error during call to poll, '{}'", netErrorString());

  if (ret == 0)
    return {};

  for (auto [index, p] : std::views::enumerate(query)) {
    if (p.first->isOpen()) {
      auto& pfd = pollfds[static_cast<std::size_t>(index)];
      SocketPollResultEntry pr;
      pr.readable = pfd.revents & POLLIN;
      pr.writable = pfd.revents & POLLOUT;
      pr.exception = pfd.revents & POLLHUP || pfd.revents & POLLNVAL || pfd.revents & POLLERR;
      if (pfd.revents & POLLHUP)
        p.first->doShutdown();
      result.add(p.first, std::move(pr));
    }
  }
#endif

  readLockers.clear();

  return result;
}

void Socket::waitForData(bool writable, std::chrono::steady_clock::time_point deadline) const {
  ReadLocker locker(m_mutex);
  if (!isActive())
    return;
  auto impl = m_impl;
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  auto wakeSocket = impl->prepareWait();
#endif
  locker.unlock();
  // Shutdown may race wake-channel initialization before native waiting begins.
  if (!isActive())
    return;

  while (true) {
    auto now = std::chrono::steady_clock::now();
    auto remaining = std::chrono::ceil<std::chrono::milliseconds>(deadline - now).count();
    auto timeout = std::clamp<std::int64_t>(remaining, 0, std::numeric_limits<int>::max());
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
    fd_set reads;
    fd_set writes;
    fd_set exceptions;
    FD_ZERO(&reads);
    FD_ZERO(&writes);
    FD_ZERO(&exceptions);
    FD_SET(wakeSocket, &reads);
    if (writable)
      FD_SET(impl->socketDesc, &writes);
    else
      FD_SET(impl->socketDesc, &reads);
    FD_SET(impl->socketDesc, &exceptions);
    timeval time;
    time.tv_sec = static_cast<long>(timeout / 1000);
    time.tv_usec = static_cast<long>((timeout % 1000) * 1000);
    int result = ::select(0, &reads, &writes, &exceptions, &time);
#else
    pollfd descriptor{};
    descriptor.fd = impl->socketDesc;
    descriptor.events = writable ? POLLOUT : POLLIN;
    int result = ::poll(&descriptor, 1, static_cast<int>(timeout));
#endif
    if (result >= 0)
      return;
    if (!netErrorInterrupt())
      throw NetworkException::format("Error waiting for socket readiness: {}", netErrorString());
    if (!isActive() || std::chrono::steady_clock::now() >= deadline)
      return;
  }
}

Socket::~Socket() {
  close();
}

void Socket::bind(Logger& logger, HostAddressWithPort const& addressWithPort) {
  WriteLocker locker(m_mutex);
  checkOpen("Socket::bind");

  struct sockaddr_storage sockAddr;
  socklen_t sockAddrLen;

  if (addressWithPort.address().mode() != m_networkMode)
    throw NetworkException("Bind address does not match socket mode");

  // Ensure quick restarts don't prevent us binding
  int set = 1;
  m_impl->setSockOpt(SOL_SOCKET, SO_REUSEADDR, (void*)&set, sizeof(int));

  m_localAddress = addressWithPort;
  setNativeFromAddress(m_localAddress, &sockAddr, &sockAddrLen);
  if (::bind(m_impl->socketDesc, (struct sockaddr*)&sockAddr, sockAddrLen) < 0)
    throw NetworkException(strf("Cannot bind socket to {}: {}", m_localAddress, netErrorString()));
  if (addressWithPort.port() == 0) {
    sockAddrLen = sizeof(sockAddr);
    if (::getsockname(m_impl->socketDesc, (struct sockaddr*)&sockAddr, &sockAddrLen) != 0)
      throw NetworkException::format("Cannot query bound socket address: {}", netErrorString());
    setAddressFromNative(m_localAddress, m_networkMode, &sockAddr);
  }

  m_socketMode = SocketMode::Bound;

  logger.debug("bind {} ({})", addressWithPort, m_impl->socketDesc);
}

void Socket::listen(int backlog) {
  WriteLocker locker(m_mutex);

  if (::listen(m_impl->socketDesc, backlog) != 0)
    throw NetworkException(strf("Could not listen on socket: '{}'", netErrorString()));
}

void Socket::setTimeout(unsigned timeout) {
  ReadLocker locker(m_mutex);
  checkOpen("Socket::setTimeout");

  void* val;
  socklen_t size;
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  val = &timeout;
  size = sizeof(timeout);
#else
  struct timeval tv;
  tv.tv_sec = timeout - timeout % 1000;
  tv.tv_usec = (timeout % 1000) * 1000;
  val = &tv;
  size = sizeof(tv);
#endif

  m_impl->setSockOpt(SOL_SOCKET, SO_RCVTIMEO, val, size);
  m_impl->setSockOpt(SOL_SOCKET, SO_SNDTIMEO, val, size);
}

void Socket::setNonBlocking(bool nonBlocking) {
  ReadLocker locker(m_mutex);
  checkOpen("Socket::setNonBlocking");
#ifdef WIN32
  unsigned long mode = nonBlocking ? 1 : 0;
  if (ioctlsocket(m_impl->socketDesc, FIONBIO, &mode) != 0)
    throw NetworkException::format("Cannot set socket non-blocking mode: {}", netErrorString());
#else
  int flags = fcntl(m_impl->socketDesc, F_GETFL, 0);
  if (flags < 0)
    throw NetworkException::format("fcntl failure getting socket flags: {}", netErrorString());
  flags = nonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
  if (fcntl(m_impl->socketDesc, F_SETFL, flags) != 0)
    throw NetworkException::format("fcntl failure setting non-blocking mode: {}", netErrorString());
#endif
}

NetworkMode Socket::networkMode() const {
  ReadLocker locker(m_mutex);
  return m_networkMode;
}

SocketMode Socket::socketMode() const {
  ReadLocker locker(m_mutex);
  return m_socketMode;
}

bool Socket::isActive() const {
  return m_socketMode > SocketMode::Shutdown;
}

bool Socket::isOpen() const {
  return m_socketMode != SocketMode::Closed;
}

void Socket::shutdown() {
  ReadLocker locker(m_mutex);
  doShutdown();
}

void Socket::close() {
  WriteLocker locker(m_mutex);
  doShutdown();
  doClose();
}

Socket::Socket(SocketType type, NetworkMode networkMode)
  : m_networkMode(networkMode), m_impl(std::make_shared<SocketImpl>()), m_socketMode(SocketMode::Closed) {
  if (m_networkMode == NetworkMode::IPv4)
    m_impl->socketDesc = ::socket(AF_INET, type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
  else
    m_impl->socketDesc = ::socket(AF_INET6, type == SocketType::Tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

  if (invalidSocketDescriptor(m_impl->socketDesc))
    throw NetworkException(strf("cannot create socket: {}", netErrorString()));

  m_socketMode = SocketMode::Shutdown;
  setTimeout(60000);
  setNonBlocking(false);
}

Socket::Socket(NetworkMode networkMode, std::shared_ptr<SocketImpl> impl, SocketMode socketMode)
  : m_networkMode(networkMode), m_impl(impl), m_socketMode(socketMode) {
  setTimeout(60000);
  setNonBlocking(false);
}

void Socket::checkOpen(char const* methodName) const {
  if (m_socketMode == SocketMode::Closed)
    throw SocketClosedException::format("Socket not open in {}", methodName);
}

void Socket::doShutdown() {
  if (m_socketMode <= SocketMode::Shutdown)
    return;

  // Set socket mode first so that if this causes an exception the exception
  // handlers know the socket is being shut down.
  m_socketMode = SocketMode::Shutdown;

  if (m_impl && !invalidSocketDescriptor(m_impl->socketDesc)) {
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
    m_impl->signalWaiters();
    ::shutdown(m_impl->socketDesc, SD_BOTH);
#else
    ::shutdown(m_impl->socketDesc, SHUT_RDWR);
#endif
  }
}

void Socket::doClose() {
  if (m_socketMode == SocketMode::Closed)
    return;

  // Set socket mode first so that if this causes an exception the exception
  // handlers know the socket is being closed.
  m_socketMode = SocketMode::Closed;

  // A readiness waiter keeps the descriptor alive until its native wait exits.
  // doShutdown above wakes it without waiting for the waiter or risking fd reuse.
  m_impl.reset();
}

}
