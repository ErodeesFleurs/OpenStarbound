//! Network host address types compatible with C++ Star::HostAddress
//!
//! This module provides IP address types for network operations.

use std::fmt;
use std::net::{IpAddr, Ipv4Addr, Ipv6Addr, SocketAddr};
use std::str::FromStr;

use serde::{Deserialize, Serialize};

use crate::error::{Error, Result};

/// Network mode (IPv4 or IPv6)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum NetworkMode {
    IPv4,
    IPv6,
}

impl Default for NetworkMode {
    fn default() -> Self {
        NetworkMode::IPv4
    }
}

/// Host address compatible with C++ Star::HostAddress
#[derive(Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct HostAddress {
    mode: NetworkMode,
    /// Address bytes - 4 for IPv4, 16 for IPv6
    address: [u8; 16],
}

impl Default for HostAddress {
    fn default() -> Self {
        Self::zero(NetworkMode::IPv4)
    }
}

impl HostAddress {
    /// Create a localhost address
    pub fn localhost(mode: NetworkMode) -> Self {
        match mode {
            NetworkMode::IPv4 => Self::from_ipv4(Ipv4Addr::LOCALHOST),
            NetworkMode::IPv6 => Self::from_ipv6(Ipv6Addr::LOCALHOST),
        }
    }

    /// Create a zero (any) address
    pub fn zero(mode: NetworkMode) -> Self {
        Self {
            mode,
            address: [0u8; 16],
        }
    }

    /// Create from an IPv4 address
    pub fn from_ipv4(addr: Ipv4Addr) -> Self {
        let mut address = [0u8; 16];
        address[..4].copy_from_slice(&addr.octets());
        Self {
            mode: NetworkMode::IPv4,
            address,
        }
    }

    /// Create from an IPv6 address
    pub fn from_ipv6(addr: Ipv6Addr) -> Self {
        let mut address = [0u8; 16];
        address.copy_from_slice(&addr.octets());
        Self {
            mode: NetworkMode::IPv6,
            address,
        }
    }

    /// Create from a generic IpAddr
    pub fn from_ip(addr: IpAddr) -> Self {
        match addr {
            IpAddr::V4(v4) => Self::from_ipv4(v4),
            IpAddr::V6(v6) => Self::from_ipv6(v6),
        }
    }

    /// Parse from a string
    pub fn parse(address: &str) -> Result<Self> {
        // Try IPv4 first
        if let Ok(addr) = address.parse::<Ipv4Addr>() {
            return Ok(Self::from_ipv4(addr));
        }
        
        // Try IPv6
        if let Ok(addr) = address.parse::<Ipv6Addr>() {
            return Ok(Self::from_ipv6(addr));
        }

        Err(Error::parse(format!("Invalid IP address: {}", address)))
    }

    /// Lookup a hostname (simplified - just parses IP for now)
    pub fn lookup(address: &str) -> Result<Self> {
        Self::parse(address)
    }

    /// Get the network mode
    pub fn mode(&self) -> NetworkMode {
        self.mode
    }

    /// Get the address bytes
    pub fn bytes(&self) -> &[u8] {
        match self.mode {
            NetworkMode::IPv4 => &self.address[..4],
            NetworkMode::IPv6 => &self.address[..16],
        }
    }

    /// Get a specific octet (with bounds checking)
    pub fn octet(&self, i: usize) -> Option<u8> {
        if i < self.size() {
            Some(self.address[i])
        } else {
            None
        }
    }

    /// Get a specific octet (panics if out of bounds)
    pub fn octet_unchecked(&self, i: usize) -> u8 {
        self.address[i]
    }

    /// Get the size in bytes
    pub fn size(&self) -> usize {
        match self.mode {
            NetworkMode::IPv4 => 4,
            NetworkMode::IPv6 => 16,
        }
    }

    /// Check if this is localhost
    pub fn is_localhost(&self) -> bool {
        match self.mode {
            NetworkMode::IPv4 => self.address[..4] == [127, 0, 0, 1],
            NetworkMode::IPv6 => {
                self.address == [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]
            }
        }
    }

    /// Check if this is a zero address
    pub fn is_zero(&self) -> bool {
        self.bytes().iter().all(|&b| b == 0)
    }

    /// Convert to standard library IpAddr
    pub fn to_ip_addr(&self) -> IpAddr {
        match self.mode {
            NetworkMode::IPv4 => IpAddr::V4(Ipv4Addr::new(
                self.address[0],
                self.address[1],
                self.address[2],
                self.address[3],
            )),
            NetworkMode::IPv6 => IpAddr::V6(Ipv6Addr::from(self.address)),
        }
    }
}

impl fmt::Display for HostAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.to_ip_addr())
    }
}

impl fmt::Debug for HostAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "HostAddress({})", self)
    }
}

impl FromStr for HostAddress {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self> {
        Self::parse(s)
    }
}

impl From<Ipv4Addr> for HostAddress {
    fn from(addr: Ipv4Addr) -> Self {
        Self::from_ipv4(addr)
    }
}

impl From<Ipv6Addr> for HostAddress {
    fn from(addr: Ipv6Addr) -> Self {
        Self::from_ipv6(addr)
    }
}

impl From<IpAddr> for HostAddress {
    fn from(addr: IpAddr) -> Self {
        Self::from_ip(addr)
    }
}

/// Host address with port compatible with C++ Star::HostAddressWithPort
#[derive(Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct HostAddressWithPort {
    address: HostAddress,
    port: u16,
}

impl Default for HostAddressWithPort {
    fn default() -> Self {
        Self {
            address: HostAddress::default(),
            port: 0,
        }
    }
}

impl HostAddressWithPort {
    /// Create a new address with port
    pub fn new(address: HostAddress, port: u16) -> Self {
        Self { address, port }
    }

    /// Parse from address string and port
    pub fn from_address_and_port(address: &str, port: u16) -> Result<Self> {
        Ok(Self {
            address: HostAddress::parse(address)?,
            port,
        })
    }

    /// Parse from a combined address:port string
    pub fn parse(address: &str) -> Result<Self> {
        // Handle IPv6 addresses with brackets: [::1]:8080
        if let Some(bracket_end) = address.find(']') {
            if !address.starts_with('[') {
                return Err(Error::parse("Invalid IPv6 address format"));
            }
            let addr_part = &address[1..bracket_end];
            let port_part = &address[bracket_end + 1..];
            
            if port_part.is_empty() {
                return Err(Error::parse("Missing port"));
            }
            if !port_part.starts_with(':') {
                return Err(Error::parse("Expected ':' after IPv6 address"));
            }
            let port: u16 = port_part[1..].parse()
                .map_err(|_| Error::parse("Invalid port number"))?;
            
            return Ok(Self {
                address: HostAddress::parse(addr_part)?,
                port,
            });
        }
        
        // Handle IPv4 addresses: 127.0.0.1:8080
        if let Some(colon_pos) = address.rfind(':') {
            let addr_part = &address[..colon_pos];
            let port_part = &address[colon_pos + 1..];
            
            let port: u16 = port_part.parse()
                .map_err(|_| Error::parse("Invalid port number"))?;
            
            return Ok(Self {
                address: HostAddress::parse(addr_part)?,
                port,
            });
        }
        
        Err(Error::parse("Missing port in address"))
    }

    /// Lookup address with separate port
    pub fn lookup(address: &str, port: u16) -> Result<Self> {
        Ok(Self {
            address: HostAddress::lookup(address)?,
            port,
        })
    }

    /// Get the address
    pub fn address(&self) -> &HostAddress {
        &self.address
    }

    /// Get the port
    pub fn port(&self) -> u16 {
        self.port
    }

    /// Convert to standard library SocketAddr
    pub fn to_socket_addr(&self) -> SocketAddr {
        SocketAddr::new(self.address.to_ip_addr(), self.port)
    }
}

impl fmt::Display for HostAddressWithPort {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.address.mode {
            NetworkMode::IPv4 => write!(f, "{}:{}", self.address, self.port),
            NetworkMode::IPv6 => write!(f, "[{}]:{}", self.address, self.port),
        }
    }
}

impl fmt::Debug for HostAddressWithPort {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "HostAddressWithPort({})", self)
    }
}

impl FromStr for HostAddressWithPort {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self> {
        Self::parse(s)
    }
}

impl From<SocketAddr> for HostAddressWithPort {
    fn from(addr: SocketAddr) -> Self {
        Self {
            address: HostAddress::from_ip(addr.ip()),
            port: addr.port(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_localhost_ipv4() {
        let addr = HostAddress::localhost(NetworkMode::IPv4);
        assert!(addr.is_localhost());
        assert_eq!(addr.mode(), NetworkMode::IPv4);
        assert_eq!(addr.to_string(), "127.0.0.1");
    }

    #[test]
    fn test_localhost_ipv6() {
        let addr = HostAddress::localhost(NetworkMode::IPv6);
        assert!(addr.is_localhost());
        assert_eq!(addr.mode(), NetworkMode::IPv6);
        assert_eq!(addr.to_string(), "::1");
    }

    #[test]
    fn test_zero_address() {
        let addr = HostAddress::zero(NetworkMode::IPv4);
        assert!(addr.is_zero());
        assert!(!addr.is_localhost());
    }

    #[test]
    fn test_parse_ipv4() {
        let addr = HostAddress::parse("192.168.1.1").unwrap();
        assert_eq!(addr.mode(), NetworkMode::IPv4);
        assert_eq!(addr.octet(0), Some(192));
        assert_eq!(addr.octet(1), Some(168));
        assert_eq!(addr.octet(2), Some(1));
        assert_eq!(addr.octet(3), Some(1));
        assert_eq!(addr.octet(4), None); // Out of bounds for IPv4
    }

    #[test]
    fn test_parse_ipv6() {
        let addr = HostAddress::parse("::1").unwrap();
        assert_eq!(addr.mode(), NetworkMode::IPv6);
        assert!(addr.is_localhost());
    }

    #[test]
    fn test_address_with_port_ipv4() {
        let addr = HostAddressWithPort::parse("192.168.1.1:8080").unwrap();
        assert_eq!(addr.port(), 8080);
        assert_eq!(addr.address().to_string(), "192.168.1.1");
    }

    #[test]
    fn test_address_with_port_ipv6() {
        let addr = HostAddressWithPort::parse("[::1]:8080").unwrap();
        assert_eq!(addr.port(), 8080);
        assert!(addr.address().is_localhost());
    }

    #[test]
    fn test_to_socket_addr() {
        let addr = HostAddressWithPort::parse("127.0.0.1:3000").unwrap();
        let socket = addr.to_socket_addr();
        assert_eq!(socket.port(), 3000);
        assert!(socket.ip().is_loopback());
    }

    #[test]
    fn test_display() {
        let addr = HostAddressWithPort::parse("10.0.0.1:443").unwrap();
        assert_eq!(addr.to_string(), "10.0.0.1:443");
        
        let addr6 = HostAddressWithPort::parse("[::1]:443").unwrap();
        assert_eq!(addr6.to_string(), "[::1]:443");
    }
}
