module;
import std;
import star.io_device;
import star.string;
import star.either;
import star.host_address;
import star.thread;
import star.socket;

namespace Star {

class Logger;

class TcpSocket : public Socket {
public:
  static std::shared_ptr<TcpSocket> connectTo(HostAddressWithPort const& address);
  static std::shared_ptr<TcpSocket> listen(Logger& logger, HostAddressWithPort const& address);

  std::shared_ptr<TcpSocket> accept(Logger& logger);

  // Must be called after connect.  Sets TCP_NODELAY option.
  void setNoDelay(bool noDelay);

  std::size_t receive(char* data, std::size_t len);
  std::size_t send(char const* data, std::size_t len);

  HostAddressWithPort localAddress() const;
  HostAddressWithPort remoteAddress() const;

private:
  TcpSocket(NetworkMode networkMode);
  TcpSocket(NetworkMode networkMode, std::shared_ptr<SocketImpl> impl);

  void connect(HostAddressWithPort const& address);

  HostAddressWithPort m_remoteAddress;
};

// Simple class to listen for and open TcpSocket instances.
class TcpServer {
public:
  typedef std::function<void(std::shared_ptr<TcpSocket> socket)> AcceptCallback;

  TcpServer(Logger& logger, HostAddressWithPort const& address);
  // Listens to all interfaces.
  TcpServer(Logger& logger, std::uint16_t port);
  ~TcpServer();

  void stop();
  bool isListening() const;

  // Blocks until next connection available for the given timeout.  Throws
  // ServerClosed if close() is called.  Cannot be called if AcceptCallback is
  // set.
  std::shared_ptr<TcpSocket> accept(unsigned timeout);

  // Rather than calling and blocking on accept(), if an AcceptCallback is set
  // here, it will be called whenever a new connection is available.
  // Exceptions thrown from the callback function will be caught and logged,
  // and will cause the server to close.  The timeout here is the timeout that
  // is passed to accept in the loop, the longer the timeout the slower it will
  // shutdown on a call to close.
  void setAcceptCallback(AcceptCallback callback, unsigned timeout = 20);

private:
  Logger& m_logger;
  mutable Mutex m_mutex;

  AcceptCallback m_callback;
  ThreadFunction<void> m_callbackThread;
  HostAddressWithPort m_hostAddress;
  std::shared_ptr<TcpSocket> m_listenSocket;
};

}

export module star.tcp;

export namespace Star {
  using ::Star::TcpSocket;
  using ::Star::TcpServer;
}
