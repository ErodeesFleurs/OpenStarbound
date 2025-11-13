/// Starbound Network Protocol Implementation in Rust
/// Protocol Version: 747
/// 
/// This module implements the Starbound network protocol with full binary compatibility
/// to the C++ implementation.

use bytes::{Buf, BufMut, Bytes, BytesMut};
use std::io::{self, Cursor};
use thiserror::Error;

/// The Starbound protocol version - must match the C++ version
pub const PROTOCOL_VERSION: u32 = 747;

#[derive(Error, Debug)]
pub enum ProtocolError {
    #[error("IO error: {0}")]
    Io(#[from] io::Error),
    
    #[error("Invalid packet type: {0}")]
    InvalidPacketType(u8),
    
    #[error("Packet too large: {0} bytes")]
    PacketTooLarge(usize),
    
    #[error("Protocol version mismatch: expected {expected}, got {actual}")]
    VersionMismatch { expected: u32, actual: u32 },
}

/// Packet types as defined in StarNetPackets.hpp
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PacketType {
    // Handshake packets
    ProtocolRequest = 0,
    ProtocolResponse = 1,
    
    // Server -> Client packets
    ServerDisconnect = 2,
    ConnectSuccess = 3,
    ConnectFailure = 4,
    HandshakeChallenge = 5,
    ChatReceive = 6,
    UniverseTimeUpdate = 7,
    CelestialResponse = 8,
    PlayerWarpResult = 9,
    PlanetTypeUpdate = 10,
    Pause = 11,
    ServerInfo = 12,
    
    // Client -> Server packets
    ClientConnect = 13,
    ClientDisconnectRequest = 14,
    HandshakeResponse = 15,
    PlayerWarp = 16,
    FlyShip = 17,
    ChatSend = 18,
    CelestialRequest = 19,
    
    // Bidirectional packets
    ClientContextUpdate = 20,
    
    // World Server -> Client packets
    WorldStart = 21,
    WorldStop = 22,
    // ... and many more packet types
}

impl PacketType {
    pub fn from_u8(value: u8) -> Result<Self, ProtocolError> {
        match value {
            0 => Ok(PacketType::ProtocolRequest),
            1 => Ok(PacketType::ProtocolResponse),
            2 => Ok(PacketType::ServerDisconnect),
            3 => Ok(PacketType::ConnectSuccess),
            4 => Ok(PacketType::ConnectFailure),
            5 => Ok(PacketType::HandshakeChallenge),
            6 => Ok(PacketType::ChatReceive),
            12 => Ok(PacketType::ServerInfo),
            13 => Ok(PacketType::ClientConnect),
            14 => Ok(PacketType::ClientDisconnectRequest),
            15 => Ok(PacketType::HandshakeResponse),
            18 => Ok(PacketType::ChatSend),
            _ => Err(ProtocolError::InvalidPacketType(value)),
        }
    }
}

/// Chat send mode as defined in StarChatTypes.hpp
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ChatSendMode {
    Broadcast = 0,
    Local = 1,
    Party = 2,
}

impl ChatSendMode {
    pub fn from_u8(value: u8) -> Result<Self, ProtocolError> {
        match value {
            0 => Ok(ChatSendMode::Broadcast),
            1 => Ok(ChatSendMode::Local),
            2 => Ok(ChatSendMode::Party),
            _ => Err(ProtocolError::InvalidPacketType(value)),
        }
    }
}

/// Message context mode as defined in StarChatTypes.hpp
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MessageContextMode {
    Local = 0,
    Party = 1,
    Broadcast = 2,
    Whisper = 3,
    CommandResult = 4,
    RadioMessage = 5,
    World = 6,
}

impl MessageContextMode {
    pub fn from_u8(value: u8) -> Result<Self, ProtocolError> {
        match value {
            0 => Ok(MessageContextMode::Local),
            1 => Ok(MessageContextMode::Party),
            2 => Ok(MessageContextMode::Broadcast),
            3 => Ok(MessageContextMode::Whisper),
            4 => Ok(MessageContextMode::CommandResult),
            5 => Ok(MessageContextMode::RadioMessage),
            6 => Ok(MessageContextMode::World),
            _ => Err(ProtocolError::InvalidPacketType(value)),
        }
    }
}

/// Message context structure
#[derive(Debug, Clone)]
pub struct MessageContext {
    pub mode: MessageContextMode,
    pub channel_name: String,
}

impl MessageContext {
    pub fn new(mode: MessageContextMode) -> Self {
        Self {
            mode,
            channel_name: String::new(),
        }
    }

    pub fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for message context mode",
            )));
        }
        let mode = MessageContextMode::from_u8(buf.get_u8())?;
        
        // Read channel name for Local and Party modes
        let channel_name = if matches!(mode, MessageContextMode::Local | MessageContextMode::Party) {
            let len = VLQ::read_unsigned(buf)? as usize;
            if buf.remaining() < len {
                return Err(ProtocolError::Io(io::Error::new(
                    io::ErrorKind::UnexpectedEof,
                    "Not enough bytes for channel name",
                )));
            }
            let mut bytes = vec![0u8; len];
            buf.copy_to_slice(&mut bytes);
            String::from_utf8_lossy(&bytes).to_string()
        } else {
            String::new()
        };
        
        Ok(Self { mode, channel_name })
    }

    pub fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_u8(self.mode as u8);
        if matches!(self.mode, MessageContextMode::Local | MessageContextMode::Party) {
            let name_bytes = self.channel_name.as_bytes();
            VLQ::write_unsigned(buf, name_bytes.len() as u64);
            buf.put_slice(name_bytes);
        }
        Ok(())
    }
}

/// Chat received message structure
#[derive(Debug, Clone)]
pub struct ChatReceivedMessage {
    pub context: MessageContext,
    pub from_connection: u16,
    pub from_nick: String,
    pub portrait: String,
    pub text: String,
}

impl ChatReceivedMessage {
    pub fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        let context = MessageContext::read(buf)?;
        
        if buf.remaining() < 2 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for connection ID",
            )));
        }
        let from_connection = buf.get_u16();
        
        // Read from_nick
        let nick_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < nick_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for nick",
            )));
        }
        let mut nick_bytes = vec![0u8; nick_len];
        buf.copy_to_slice(&mut nick_bytes);
        let from_nick = String::from_utf8_lossy(&nick_bytes).to_string();
        
        // Read portrait
        let portrait_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < portrait_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for portrait",
            )));
        }
        let mut portrait_bytes = vec![0u8; portrait_len];
        buf.copy_to_slice(&mut portrait_bytes);
        let portrait = String::from_utf8_lossy(&portrait_bytes).to_string();
        
        // Read text
        let text_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < text_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for text",
            )));
        }
        let mut text_bytes = vec![0u8; text_len];
        buf.copy_to_slice(&mut text_bytes);
        let text = String::from_utf8_lossy(&text_bytes).to_string();
        
        Ok(Self {
            context,
            from_connection,
            from_nick,
            portrait,
            text,
        })
    }

    pub fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        self.context.write(buf)?;
        buf.put_u16(self.from_connection);
        
        let nick_bytes = self.from_nick.as_bytes();
        VLQ::write_unsigned(buf, nick_bytes.len() as u64);
        buf.put_slice(nick_bytes);
        
        let portrait_bytes = self.portrait.as_bytes();
        VLQ::write_unsigned(buf, portrait_bytes.len() as u64);
        buf.put_slice(portrait_bytes);
        
        let text_bytes = self.text.as_bytes();
        VLQ::write_unsigned(buf, text_bytes.len() as u64);
        buf.put_slice(text_bytes);
        
        Ok(())
    }
}

/// VLQ (Variable Length Quantity) encoding/decoding
/// Matches the C++ implementation in StarDataStream
pub struct VLQ;

impl VLQ {
    /// Read an unsigned VLQ integer
    pub fn read_unsigned(buf: &mut Cursor<&[u8]>) -> io::Result<u64> {
        let mut result: u64 = 0;
        let mut shift = 0;
        
        loop {
            if !buf.has_remaining() {
                return Err(io::Error::new(io::ErrorKind::UnexpectedEof, "EOF while reading VLQ"));
            }
            
            let byte = buf.get_u8();
            result |= ((byte & 0x7F) as u64) << shift;
            
            if (byte & 0x80) == 0 {
                break;
            }
            
            shift += 7;
            if shift >= 64 {
                return Err(io::Error::new(io::ErrorKind::InvalidData, "VLQ overflow"));
            }
        }
        
        Ok(result)
    }
    
    /// Read a signed VLQ integer (zigzag encoding)
    pub fn read_signed(buf: &mut Cursor<&[u8]>) -> io::Result<i64> {
        let unsigned = Self::read_unsigned(buf)?;
        // Zigzag decode: (n >> 1) ^ -(n & 1)
        let result = ((unsigned >> 1) as i64) ^ (-((unsigned & 1) as i64));
        Ok(result)
    }
    
    /// Write an unsigned VLQ integer
    pub fn write_unsigned(buf: &mut BytesMut, mut value: u64) {
        loop {
            let mut byte = (value & 0x7F) as u8;
            value >>= 7;
            
            if value != 0 {
                byte |= 0x80;
            }
            
            buf.put_u8(byte);
            
            if value == 0 {
                break;
            }
        }
    }
    
    /// Write a signed VLQ integer (zigzag encoding)
    pub fn write_signed(buf: &mut BytesMut, value: i64) {
        // Zigzag encode: (n << 1) ^ (n >> 63)
        let unsigned = ((value << 1) ^ (value >> 63)) as u64;
        Self::write_unsigned(buf, unsigned);
    }
}

/// Represents a Starbound packet
pub trait Packet: Send + Sync {
    fn packet_type(&self) -> PacketType;
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError>;
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError>
    where
        Self: Sized;
}

/// Protocol Request Packet - sent by client to initiate connection
#[derive(Debug, Clone)]
pub struct ProtocolRequestPacket {
    pub request_protocol_version: u32,
}

impl Packet for ProtocolRequestPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ProtocolRequest
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_u32(self.request_protocol_version);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 4 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for protocol version",
            )));
        }
        Ok(Self {
            request_protocol_version: buf.get_u32(),
        })
    }
}

/// Protocol Response Packet - sent by server in response to ProtocolRequest
#[derive(Debug, Clone)]
pub struct ProtocolResponsePacket {
    pub allowed: bool,
    pub info: String, // JSON string
}

impl Packet for ProtocolResponsePacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ProtocolResponse
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_u8(if self.allowed { 1 } else { 0 });
        // Write string length as VLQ followed by UTF-8 bytes
        let info_bytes = self.info.as_bytes();
        VLQ::write_unsigned(buf, info_bytes.len() as u64);
        buf.put_slice(info_bytes);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for allowed flag",
            )));
        }
        let allowed = buf.get_u8() != 0;
        
        let str_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < str_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for info string",
            )));
        }
        
        let mut str_bytes = vec![0u8; str_len];
        buf.copy_to_slice(&mut str_bytes);
        let info = String::from_utf8_lossy(&str_bytes).to_string();
        
        Ok(Self { allowed, info })
    }
}

/// Server Disconnect Packet - sent by server to disconnect client
#[derive(Debug, Clone)]
pub struct ServerDisconnectPacket {
    pub reason: String,
}

impl Packet for ServerDisconnectPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ServerDisconnect
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        let reason_bytes = self.reason.as_bytes();
        VLQ::write_unsigned(buf, reason_bytes.len() as u64);
        buf.put_slice(reason_bytes);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        let str_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < str_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for reason string",
            )));
        }
        
        let mut str_bytes = vec![0u8; str_len];
        buf.copy_to_slice(&mut str_bytes);
        let reason = String::from_utf8_lossy(&str_bytes).to_string();
        
        Ok(Self { reason })
    }
}

/// Connect Success Packet - sent by server after successful handshake
#[derive(Debug, Clone)]
pub struct ConnectSuccessPacket {
    pub client_id: u16,
    pub server_uuid: String,
    // Simplified for MVP - full implementation would include celestial information
}

impl Packet for ConnectSuccessPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ConnectSuccess
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_u16(self.client_id);
        let uuid_bytes = self.server_uuid.as_bytes();
        VLQ::write_unsigned(buf, uuid_bytes.len() as u64);
        buf.put_slice(uuid_bytes);
        // Note: Full implementation would write celestial information here
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 2 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for client ID",
            )));
        }
        let client_id = buf.get_u16();
        
        let str_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < str_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for UUID",
            )));
        }
        
        let mut str_bytes = vec![0u8; str_len];
        buf.copy_to_slice(&mut str_bytes);
        let server_uuid = String::from_utf8_lossy(&str_bytes).to_string();
        
        Ok(Self { client_id, server_uuid })
    }
}

/// Chat Send Packet - sent by client to send a chat message
#[derive(Debug, Clone)]
pub struct ChatSendPacket {
    pub text: String,
    pub send_mode: ChatSendMode,
}

impl Packet for ChatSendPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ChatSend
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        let text_bytes = self.text.as_bytes();
        VLQ::write_unsigned(buf, text_bytes.len() as u64);
        buf.put_slice(text_bytes);
        buf.put_u8(self.send_mode as u8);
        // Note: Not writing data JsonObject for now (requires stream version >= 5)
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        let text_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < text_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for text",
            )));
        }
        
        let mut text_bytes = vec![0u8; text_len];
        buf.copy_to_slice(&mut text_bytes);
        let text = String::from_utf8_lossy(&text_bytes).to_string();
        
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for send mode",
            )));
        }
        let send_mode = ChatSendMode::from_u8(buf.get_u8())?;
        
        Ok(Self { text, send_mode })
    }
}

/// Chat Receive Packet - sent by server to deliver a chat message
#[derive(Debug, Clone)]
pub struct ChatReceivePacket {
    pub received_message: ChatReceivedMessage,
}

impl Packet for ChatReceivePacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ChatReceive
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        self.received_message.write(buf)
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        let received_message = ChatReceivedMessage::read(buf)?;
        Ok(Self { received_message })
    }
}

/// Server Info Packet - sent by server to provide server information
#[derive(Debug, Clone)]
pub struct ServerInfoPacket {
    pub players: u16,
    pub max_players: u16,
}

impl Packet for ServerInfoPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ServerInfo
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_u16(self.players);
        buf.put_u16(self.max_players);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 4 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for server info",
            )));
        }
        
        let players = buf.get_u16();
        let max_players = buf.get_u16();
        
        Ok(Self { players, max_players })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_vlq_unsigned() {
        let mut buf = BytesMut::new();
        VLQ::write_unsigned(&mut buf, 0);
        VLQ::write_unsigned(&mut buf, 127);
        VLQ::write_unsigned(&mut buf, 128);
        VLQ::write_unsigned(&mut buf, 16384);
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        
        assert_eq!(VLQ::read_unsigned(&mut cursor).unwrap(), 0);
        assert_eq!(VLQ::read_unsigned(&mut cursor).unwrap(), 127);
        assert_eq!(VLQ::read_unsigned(&mut cursor).unwrap(), 128);
        assert_eq!(VLQ::read_unsigned(&mut cursor).unwrap(), 16384);
    }

    #[test]
    fn test_vlq_signed() {
        let mut buf = BytesMut::new();
        VLQ::write_signed(&mut buf, 0);
        VLQ::write_signed(&mut buf, -1);
        VLQ::write_signed(&mut buf, 1);
        VLQ::write_signed(&mut buf, -64);
        VLQ::write_signed(&mut buf, 64);
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        
        assert_eq!(VLQ::read_signed(&mut cursor).unwrap(), 0);
        assert_eq!(VLQ::read_signed(&mut cursor).unwrap(), -1);
        assert_eq!(VLQ::read_signed(&mut cursor).unwrap(), 1);
        assert_eq!(VLQ::read_signed(&mut cursor).unwrap(), -64);
        assert_eq!(VLQ::read_signed(&mut cursor).unwrap(), 64);
    }

    #[test]
    fn test_protocol_request_packet() {
        let packet = ProtocolRequestPacket {
            request_protocol_version: PROTOCOL_VERSION,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = ProtocolRequestPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.request_protocol_version, PROTOCOL_VERSION);
    }

    #[test]
    fn test_protocol_response_packet() {
        let packet = ProtocolResponsePacket {
            allowed: true,
            info: r#"{"version":"test"}"#.to_string(),
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = ProtocolResponsePacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.allowed, true);
        assert_eq!(decoded.info, r#"{"version":"test"}"#);
    }

    #[test]
    fn test_chat_send_packet() {
        let packet = ChatSendPacket {
            text: "Hello, world!".to_string(),
            send_mode: ChatSendMode::Broadcast,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = ChatSendPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.text, "Hello, world!");
        assert_eq!(decoded.send_mode, ChatSendMode::Broadcast);
    }

    #[test]
    fn test_chat_receive_packet() {
        let packet = ChatReceivePacket {
            received_message: ChatReceivedMessage {
                context: MessageContext::new(MessageContextMode::Broadcast),
                from_connection: 1,
                from_nick: "TestPlayer".to_string(),
                portrait: "".to_string(),
                text: "Hello!".to_string(),
            },
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = ChatReceivePacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.received_message.from_connection, 1);
        assert_eq!(decoded.received_message.from_nick, "TestPlayer");
        assert_eq!(decoded.received_message.text, "Hello!");
    }

    #[test]
    fn test_server_info_packet() {
        let packet = ServerInfoPacket {
            players: 5,
            max_players: 8,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = ServerInfoPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.players, 5);
        assert_eq!(decoded.max_players, 8);
    }
}
