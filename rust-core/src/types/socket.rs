//! Network socket utilities compatible with C++ Star::Socket, Star::TcpSocket, Star::UdpSocket.
//!
//! This module provides TCP and UDP socket types that match the C++ implementation.

use crate::types::host_address::{HostAddress, HostAddressWithPort, NetworkMode};
use crate::Error;
use std::io::{self, Read, Write};
use std::net::{
    Shutdown, SocketAddr, TcpListener, TcpStream, UdpSocket as StdUdpSocket,
};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::Duration;

/// Socket mode indicating the current state of the socket.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SocketMode {
    /// Socket is closed
    Closed,
    /// Socket is shutdown (no longer usable but not yet closed)
    Shutdown,
    /// Socket is bound to an address (server listening)
    Bound,
    /// Socket is connected to a remote address
    Connected,
}

/// Maximum recommended UDP datagram size without fragmentation.
pub const MAX_UDP_DATA: usize = 1460;

/// Default socket timeout in milliseconds.
pub const DEFAULT_SOCKET_TIMEOUT_MS: u64 = 60000;

/// TCP socket for stream-based communication.
///
/// Provides reliable, ordered, and error-checked delivery of a stream of bytes.
pub struct TcpSocket {
    stream: Option<TcpStream>,
    mode: SocketMode,
    local_address: Option<HostAddressWithPort>,
    remote_address: Option<HostAddressWithPort>,
    non_blocking: bool,
    timeout_ms: u64,
}

impl TcpSocket {
    /// Connect to a remote address.
    ///
    /// # Arguments
    /// * `address` - The address to connect to
    ///
    /// # Returns
    /// A connected TCP socket
    pub fn connect(address: &HostAddressWithPort) -> Result<Self, Error> {
        let addr = address.to_socket_addr();
        let stream = TcpStream::connect(addr).map_err(|e| Error::Network(e.to_string()))?;

        let local_addr = stream
            .local_addr()
            .ok()
            .map(HostAddressWithPort::from);
        let remote_addr = Some(address.clone());

        // Set default timeout
        let _ = stream.set_read_timeout(Some(Duration::from_millis(DEFAULT_SOCKET_TIMEOUT_MS)));
        let _ = stream.set_write_timeout(Some(Duration::from_millis(DEFAULT_SOCKET_TIMEOUT_MS)));

        Ok(TcpSocket {
            stream: Some(stream),
            mode: SocketMode::Connected,
            local_address: local_addr,
            remote_address: remote_addr,
            non_blocking: false,
            timeout_ms: DEFAULT_SOCKET_TIMEOUT_MS,
        })
    }

    /// Connect with a timeout.
    ///
    /// # Arguments
    /// * `address` - The address to connect to
    /// * `timeout_ms` - Connection timeout in milliseconds
    pub fn connect_timeout(address: &HostAddressWithPort, timeout_ms: u64) -> Result<Self, Error> {
        let addr = address.to_socket_addr();
        let stream = TcpStream::connect_timeout(&addr, Duration::from_millis(timeout_ms))
            .map_err(|e| Error::Network(e.to_string()))?;

        let local_addr = stream
            .local_addr()
            .ok()
            .map(HostAddressWithPort::from);
        let remote_addr = Some(address.clone());

        let _ = stream.set_read_timeout(Some(Duration::from_millis(timeout_ms)));
        let _ = stream.set_write_timeout(Some(Duration::from_millis(timeout_ms)));

        Ok(TcpSocket {
            stream: Some(stream),
            mode: SocketMode::Connected,
            local_address: local_addr,
            remote_address: remote_addr,
            non_blocking: false,
            timeout_ms,
        })
    }

    /// Create a listening socket.
    ///
    /// # Arguments
    /// * `address` - The address to bind and listen on
    pub fn listen(address: &HostAddressWithPort) -> Result<TcpServer, Error> {
        TcpServer::new(address)
    }

    /// Set non-blocking mode.
    ///
    /// # Arguments
    /// * `non_blocking` - Whether to use non-blocking I/O
    pub fn set_non_blocking(&mut self, non_blocking: bool) -> Result<(), Error> {
        if let Some(ref stream) = self.stream {
            stream
                .set_nonblocking(non_blocking)
                .map_err(|e| Error::Network(e.to_string()))?;
            self.non_blocking = non_blocking;
        }
        Ok(())
    }

    /// Set socket timeout in milliseconds.
    ///
    /// # Arguments
    /// * `millis` - Timeout in milliseconds
    pub fn set_timeout(&mut self, millis: u64) {
        self.timeout_ms = millis;
        if let Some(ref stream) = self.stream {
            let timeout = if millis == 0 {
                None
            } else {
                Some(Duration::from_millis(millis))
            };
            let _ = stream.set_read_timeout(timeout);
            let _ = stream.set_write_timeout(timeout);
        }
    }

    /// Set TCP_NODELAY option (Nagle's algorithm).
    ///
    /// When set to `true`, disables Nagle's algorithm for lower latency.
    ///
    /// # Arguments
    /// * `no_delay` - Whether to disable Nagle's algorithm
    pub fn set_no_delay(&mut self, no_delay: bool) -> Result<(), Error> {
        if let Some(ref stream) = self.stream {
            stream
                .set_nodelay(no_delay)
                .map_err(|e| Error::Network(e.to_string()))?;
        }
        Ok(())
    }

    /// Get the network mode (IPv4 or IPv6).
    pub fn network_mode(&self) -> NetworkMode {
        self.local_address
            .as_ref()
            .map(|addr| addr.address().mode())
            .unwrap_or(NetworkMode::IPv4)
    }

    /// Get the current socket mode.
    pub fn socket_mode(&self) -> SocketMode {
        self.mode
    }

    /// Check if the socket is active (bound or connected).
    pub fn is_active(&self) -> bool {
        matches!(self.mode, SocketMode::Bound | SocketMode::Connected)
    }

    /// Check if the socket is open (not closed).
    pub fn is_open(&self) -> bool {
        !matches!(self.mode, SocketMode::Closed)
    }

    /// Get the local address.
    pub fn local_address(&self) -> Option<&HostAddressWithPort> {
        self.local_address.as_ref()
    }

    /// Get the remote address.
    pub fn remote_address(&self) -> Option<&HostAddressWithPort> {
        self.remote_address.as_ref()
    }

    /// Receive data from the socket.
    ///
    /// # Arguments
    /// * `buffer` - Buffer to read into
    ///
    /// # Returns
    /// Number of bytes received
    pub fn receive(&mut self, buffer: &mut [u8]) -> Result<usize, Error> {
        if let Some(ref mut stream) = self.stream {
            stream.read(buffer).map_err(|e| {
                if e.kind() == io::ErrorKind::ConnectionReset
                    || e.kind() == io::ErrorKind::BrokenPipe
                {
                    self.mode = SocketMode::Shutdown;
                }
                Error::Network(e.to_string())
            })
        } else {
            Err(Error::Network("Socket is closed".into()))
        }
    }

    /// Send data on the socket.
    ///
    /// # Arguments
    /// * `data` - Data to send
    ///
    /// # Returns
    /// Number of bytes sent
    pub fn send(&mut self, data: &[u8]) -> Result<usize, Error> {
        if let Some(ref mut stream) = self.stream {
            stream.write(data).map_err(|e| {
                if e.kind() == io::ErrorKind::ConnectionReset
                    || e.kind() == io::ErrorKind::BrokenPipe
                {
                    self.mode = SocketMode::Shutdown;
                }
                Error::Network(e.to_string())
            })
        } else {
            Err(Error::Network("Socket is closed".into()))
        }
    }

    /// Send all data on the socket.
    ///
    /// # Arguments
    /// * `data` - Data to send
    pub fn send_all(&mut self, data: &[u8]) -> Result<(), Error> {
        if let Some(ref mut stream) = self.stream {
            stream.write_all(data).map_err(|e| {
                if e.kind() == io::ErrorKind::ConnectionReset
                    || e.kind() == io::ErrorKind::BrokenPipe
                {
                    self.mode = SocketMode::Shutdown;
                }
                Error::Network(e.to_string())
            })
        } else {
            Err(Error::Network("Socket is closed".into()))
        }
    }

    /// Shutdown the socket.
    pub fn shutdown(&mut self) {
        if let Some(ref stream) = self.stream {
            let _ = stream.shutdown(Shutdown::Both);
        }
        self.mode = SocketMode::Shutdown;
    }

    /// Close the socket.
    pub fn close(&mut self) {
        if let Some(ref stream) = self.stream {
            let _ = stream.shutdown(Shutdown::Both);
        }
        self.stream = None;
        self.mode = SocketMode::Closed;
    }
}

impl Drop for TcpSocket {
    fn drop(&mut self) {
        self.close();
    }
}

/// TCP server for accepting incoming connections.
pub struct TcpServer {
    listener: Option<TcpListener>,
    address: HostAddressWithPort,
    is_listening: Arc<AtomicBool>,
}

impl TcpServer {
    /// Create a new TCP server listening on the given address.
    ///
    /// # Arguments
    /// * `address` - The address to listen on
    pub fn new(address: &HostAddressWithPort) -> Result<Self, Error> {
        let addr = address.to_socket_addr();
        let listener = TcpListener::bind(addr).map_err(|e| Error::Network(e.to_string()))?;

        Ok(TcpServer {
            listener: Some(listener),
            address: address.clone(),
            is_listening: Arc::new(AtomicBool::new(true)),
        })
    }

    /// Create a TCP server listening on all interfaces on the given port.
    ///
    /// # Arguments
    /// * `port` - The port to listen on
    pub fn on_port(port: u16) -> Result<Self, Error> {
        let address = HostAddressWithPort::new(HostAddress::zero(NetworkMode::IPv4), port);
        Self::new(&address)
    }

    /// Check if the server is listening.
    pub fn is_listening(&self) -> bool {
        self.is_listening.load(Ordering::SeqCst) && self.listener.is_some()
    }

    /// Stop the server.
    pub fn stop(&mut self) {
        self.is_listening.store(false, Ordering::SeqCst);
        self.listener = None;
    }

    /// Accept a new connection with timeout.
    ///
    /// # Arguments
    /// * `timeout_ms` - Timeout in milliseconds
    ///
    /// # Returns
    /// The accepted TCP socket, or None if timeout
    pub fn accept(&self, timeout_ms: u64) -> Result<Option<TcpSocket>, Error> {
        if let Some(ref listener) = self.listener {
            listener
                .set_nonblocking(true)
                .map_err(|e| Error::Network(e.to_string()))?;

            let start = std::time::Instant::now();
            let timeout = Duration::from_millis(timeout_ms);

            loop {
                match listener.accept() {
                    Ok((stream, addr)) => {
                        let _ = stream.set_nonblocking(false);
                        let _ = stream.set_read_timeout(Some(Duration::from_millis(
                            DEFAULT_SOCKET_TIMEOUT_MS,
                        )));
                        let _ = stream.set_write_timeout(Some(Duration::from_millis(
                            DEFAULT_SOCKET_TIMEOUT_MS,
                        )));

                        let local_addr = stream
                            .local_addr()
                            .ok()
                            .map(HostAddressWithPort::from);
                        let remote_addr = Some(HostAddressWithPort::from(addr));

                        return Ok(Some(TcpSocket {
                            stream: Some(stream),
                            mode: SocketMode::Connected,
                            local_address: local_addr,
                            remote_address: remote_addr,
                            non_blocking: false,
                            timeout_ms: DEFAULT_SOCKET_TIMEOUT_MS,
                        }));
                    }
                    Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                        if start.elapsed() >= timeout {
                            return Ok(None);
                        }
                        std::thread::sleep(Duration::from_millis(10));
                    }
                    Err(e) => return Err(Error::Network(e.to_string())),
                }

                if !self.is_listening.load(Ordering::SeqCst) {
                    return Err(Error::Network("Server stopped".into()));
                }
            }
        } else {
            Err(Error::Network("Server is not listening".into()))
        }
    }

    /// Get the address the server is listening on.
    pub fn address(&self) -> &HostAddressWithPort {
        &self.address
    }
}

impl Drop for TcpServer {
    fn drop(&mut self) {
        self.stop();
    }
}

/// UDP socket for datagram-based communication.
///
/// Provides connectionless, unreliable delivery of individual datagrams.
pub struct UdpSocket {
    socket: Option<StdUdpSocket>,
    mode: SocketMode,
    local_address: Option<HostAddressWithPort>,
    network_mode: NetworkMode,
}

impl UdpSocket {
    /// Create a new UDP socket.
    ///
    /// # Arguments
    /// * `network_mode` - Whether to use IPv4 or IPv6
    pub fn new(network_mode: NetworkMode) -> Result<Self, Error> {
        let addr: SocketAddr = match network_mode {
            NetworkMode::IPv4 => "0.0.0.0:0".parse().unwrap(),
            NetworkMode::IPv6 => "[::]:0".parse().unwrap(),
        };

        let socket = StdUdpSocket::bind(addr).map_err(|e| Error::Network(e.to_string()))?;

        let local_addr = socket
            .local_addr()
            .ok()
            .map(HostAddressWithPort::from);

        Ok(UdpSocket {
            socket: Some(socket),
            mode: SocketMode::Bound,
            local_address: local_addr,
            network_mode,
        })
    }

    /// Bind to a specific address.
    ///
    /// # Arguments
    /// * `address` - The address to bind to
    pub fn bind(address: &HostAddressWithPort) -> Result<Self, Error> {
        let addr = address.to_socket_addr();
        let socket = StdUdpSocket::bind(addr).map_err(|e| Error::Network(e.to_string()))?;

        let network_mode = address.address().mode();

        Ok(UdpSocket {
            socket: Some(socket),
            mode: SocketMode::Bound,
            local_address: Some(address.clone()),
            network_mode,
        })
    }

    /// Set non-blocking mode.
    pub fn set_non_blocking(&mut self, non_blocking: bool) -> Result<(), Error> {
        if let Some(ref socket) = self.socket {
            socket
                .set_nonblocking(non_blocking)
                .map_err(|e| Error::Network(e.to_string()))?;
        }
        Ok(())
    }

    /// Set socket timeout.
    pub fn set_timeout(&mut self, millis: u64) -> Result<(), Error> {
        if let Some(ref socket) = self.socket {
            let timeout = if millis == 0 {
                None
            } else {
                Some(Duration::from_millis(millis))
            };
            socket
                .set_read_timeout(timeout)
                .map_err(|e| Error::Network(e.to_string()))?;
            socket
                .set_write_timeout(timeout)
                .map_err(|e| Error::Network(e.to_string()))?;
        }
        Ok(())
    }

    /// Get the network mode.
    pub fn network_mode(&self) -> NetworkMode {
        self.network_mode
    }

    /// Get the socket mode.
    pub fn socket_mode(&self) -> SocketMode {
        self.mode
    }

    /// Get the local address.
    pub fn local_address(&self) -> Option<&HostAddressWithPort> {
        self.local_address.as_ref()
    }

    /// Receive a datagram.
    ///
    /// # Arguments
    /// * `buffer` - Buffer to read into
    ///
    /// # Returns
    /// Tuple of (bytes received, sender address)
    pub fn receive(&self, buffer: &mut [u8]) -> Result<(usize, HostAddressWithPort), Error> {
        if let Some(ref socket) = self.socket {
            let (size, addr) = socket
                .recv_from(buffer)
                .map_err(|e| Error::Network(e.to_string()))?;

            Ok((size, HostAddressWithPort::from(addr)))
        } else {
            Err(Error::Network("Socket is closed".into()))
        }
    }

    /// Send a datagram to the given address.
    ///
    /// # Arguments
    /// * `address` - The destination address
    /// * `data` - Data to send
    ///
    /// # Returns
    /// Number of bytes sent
    pub fn send(&self, address: &HostAddressWithPort, data: &[u8]) -> Result<usize, Error> {
        if let Some(ref socket) = self.socket {
            let addr = address.to_socket_addr();
            socket
                .send_to(data, addr)
                .map_err(|e| Error::Network(e.to_string()))
        } else {
            Err(Error::Network("Socket is closed".into()))
        }
    }

    /// Close the socket.
    pub fn close(&mut self) {
        self.socket = None;
        self.mode = SocketMode::Closed;
    }
}

/// UDP server for receiving datagrams.
pub struct UdpServer {
    socket: UdpSocket,
    address: HostAddressWithPort,
}

impl UdpServer {
    /// Create a new UDP server.
    ///
    /// # Arguments
    /// * `address` - The address to bind to
    pub fn new(address: &HostAddressWithPort) -> Result<Self, Error> {
        let socket = UdpSocket::bind(address)?;

        Ok(UdpServer {
            socket,
            address: address.clone(),
        })
    }

    /// Check if the server is listening.
    pub fn is_listening(&self) -> bool {
        self.socket.socket_mode() == SocketMode::Bound
    }

    /// Close the server.
    pub fn close(&mut self) {
        self.socket.close();
    }

    /// Receive a datagram with timeout.
    ///
    /// # Arguments
    /// * `buffer` - Buffer to read into
    /// * `timeout_ms` - Timeout in milliseconds
    ///
    /// # Returns
    /// Tuple of (bytes received, sender address), or None on timeout
    pub fn receive(
        &mut self,
        buffer: &mut [u8],
        timeout_ms: u64,
    ) -> Result<Option<(usize, HostAddressWithPort)>, Error> {
        self.socket.set_timeout(timeout_ms)?;

        match self.socket.receive(buffer) {
            Ok(result) => Ok(Some(result)),
            Err(Error::Network(msg)) if msg.contains("timed out") || msg.contains("WouldBlock") => Ok(None),
            Err(e) => Err(e),
        }
    }

    /// Send a datagram.
    ///
    /// # Arguments
    /// * `address` - The destination address
    /// * `data` - Data to send
    ///
    /// # Returns
    /// Number of bytes sent
    pub fn send(&self, address: &HostAddressWithPort, data: &[u8]) -> Result<usize, Error> {
        self.socket.send(address, data)
    }

    /// Get the address the server is bound to.
    pub fn address(&self) -> &HostAddressWithPort {
        &self.address
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_socket_mode() {
        assert_ne!(SocketMode::Closed, SocketMode::Connected);
        assert_eq!(SocketMode::Closed, SocketMode::Closed);
    }

    #[test]
    fn test_max_udp_data() {
        assert_eq!(MAX_UDP_DATA, 1460);
    }

    #[test]
    fn test_udp_socket_create() {
        let socket = UdpSocket::new(NetworkMode::IPv4);
        assert!(socket.is_ok());

        let socket = socket.unwrap();
        assert_eq!(socket.socket_mode(), SocketMode::Bound);
        assert_eq!(socket.network_mode(), NetworkMode::IPv4);
        assert!(socket.local_address().is_some());
    }

    #[test]
    fn test_udp_socket_bind() {
        let addr = HostAddressWithPort::new(HostAddress::zero(NetworkMode::IPv4), 0);
        let socket = UdpSocket::bind(&addr);
        assert!(socket.is_ok());
    }

    #[test]
    fn test_tcp_server_create() {
        let addr = HostAddressWithPort::new(HostAddress::localhost(NetworkMode::IPv4), 0);
        let server = TcpServer::new(&addr);
        assert!(server.is_ok());

        let server = server.unwrap();
        assert!(server.is_listening());
    }

    #[test]
    fn test_tcp_server_on_port() {
        let server = TcpServer::on_port(0);
        assert!(server.is_ok());

        let server = server.unwrap();
        assert!(server.is_listening());
    }
}
