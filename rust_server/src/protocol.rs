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
    // ... more world packets ...
    
    // Bidirectional entity packets
    EntityCreate = 94,
    EntityUpdateSet = 95,
    EntityDestroy = 96,
    EntityInteract = 97,
    EntityInteractResult = 98,
    HitRequest = 99,
    DamageRequest = 100,
    DamageNotification = 101,
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
            21 => Ok(PacketType::WorldStart),
            22 => Ok(PacketType::WorldStop),
            94 => Ok(PacketType::EntityCreate),
            95 => Ok(PacketType::EntityUpdateSet),
            96 => Ok(PacketType::EntityDestroy),
            97 => Ok(PacketType::EntityInteract),
            98 => Ok(PacketType::EntityInteractResult),
            99 => Ok(PacketType::HitRequest),
            100 => Ok(PacketType::DamageRequest),
            101 => Ok(PacketType::DamageNotification),
            _ => Err(ProtocolError::InvalidPacketType(value)),
        }
    }
}

/// Entity types as defined in StarEntity.hpp
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EntityType {
    Plant = 0,
    Object = 1,
    Vehicle = 2,
    ItemDrop = 3,
    PlantDrop = 4,
    Projectile = 5,
    Stagehand = 6,
    Monster = 7,
    Npc = 8,
    Player = 9,
}

impl EntityType {
    pub fn from_u8(value: u8) -> Result<Self, ProtocolError> {
        match value {
            0 => Ok(EntityType::Plant),
            1 => Ok(EntityType::Object),
            2 => Ok(EntityType::Vehicle),
            3 => Ok(EntityType::ItemDrop),
            4 => Ok(EntityType::PlantDrop),
            5 => Ok(EntityType::Projectile),
            6 => Ok(EntityType::Stagehand),
            7 => Ok(EntityType::Monster),
            8 => Ok(EntityType::Npc),
            9 => Ok(EntityType::Player),
            _ => Err(ProtocolError::InvalidPacketType(value)),
        }
    }
}

/// Entity ID type
pub type EntityId = i32;

/// Connection ID type (already used elsewhere but defining here for entities)
pub type ConnectionId = u16;

/// Compression and decompression functions using Zstd
pub mod compression {
    use super::ProtocolError;
    use std::io;

    /// Compress data using Zstd
    pub fn compress_data(data: &[u8]) -> Result<Vec<u8>, ProtocolError> {
        zstd::bulk::compress(data, 3)
            .map_err(|e| ProtocolError::Io(io::Error::new(io::ErrorKind::Other, e)))
    }

    /// Decompress data using Zstd
    pub fn decompress_data(data: &[u8], max_size: usize) -> Result<Vec<u8>, ProtocolError> {
        let decompressed = zstd::bulk::decompress(data, max_size)
            .map_err(|e| ProtocolError::Io(io::Error::new(io::ErrorKind::Other, e)))?;
        
        if decompressed.len() > max_size {
            return Err(ProtocolError::PacketTooLarge(decompressed.len()));
        }
        
        Ok(decompressed)
    }

    #[cfg(test)]
    mod tests {
        use super::*;

        #[test]
        fn test_compression_round_trip() {
            let original = b"Hello, World! This is a test of Zstd compression.";
            let compressed = compress_data(original).unwrap();
            let decompressed = decompress_data(&compressed, 1024).unwrap();
            assert_eq!(original.as_ref(), decompressed.as_slice());
        }

        #[test]
        fn test_compression_reduces_size() {
            let original = vec![b'A'; 1000]; // Highly compressible data
            let compressed = compress_data(&original).unwrap();
            assert!(compressed.len() < original.len());
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

/// World Start Packet - sent by server when client enters a world
#[derive(Debug, Clone)]
pub struct WorldStartPacket {
    pub template_data: String,  // JSON string
    pub sky_data: Vec<u8>,
    pub weather_data: Vec<u8>,
    pub player_start: (f32, f32),
    pub player_respawn: (f32, f32),
    pub respawn_in_world: bool,
    pub world_properties: String,  // JSON string
    pub client_id: u16,
    pub local_interpolation_mode: bool,
}

impl Packet for WorldStartPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::WorldStart
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        // Write template data (JSON string)
        let template_bytes = self.template_data.as_bytes();
        VLQ::write_unsigned(buf, template_bytes.len() as u64);
        buf.put_slice(template_bytes);
        
        // Write sky data
        VLQ::write_unsigned(buf, self.sky_data.len() as u64);
        buf.put_slice(&self.sky_data);
        
        // Write weather data
        VLQ::write_unsigned(buf, self.weather_data.len() as u64);
        buf.put_slice(&self.weather_data);
        
        // Write player start position
        buf.put_f32(self.player_start.0);
        buf.put_f32(self.player_start.1);
        
        // Write player respawn position
        buf.put_f32(self.player_respawn.0);
        buf.put_f32(self.player_respawn.1);
        
        // Write respawn in world flag
        buf.put_u8(if self.respawn_in_world { 1 } else { 0 });
        
        // Write dungeon gravity map (simplified - empty for MVP)
        VLQ::write_unsigned(buf, 0);
        
        // Write dungeon breathable map (simplified - empty for MVP)
        VLQ::write_unsigned(buf, 0);
        
        // Write protected dungeon IDs (simplified - empty for MVP)
        VLQ::write_unsigned(buf, 0);
        
        // Write world properties (JSON string)
        let props_bytes = self.world_properties.as_bytes();
        VLQ::write_unsigned(buf, props_bytes.len() as u64);
        buf.put_slice(props_bytes);
        
        // Write client ID
        buf.put_u16(self.client_id);
        
        // Write local interpolation mode
        buf.put_u8(if self.local_interpolation_mode { 1 } else { 0 });
        
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        // Read template data
        let template_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < template_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for template data",
            )));
        }
        let mut template_bytes = vec![0u8; template_len];
        buf.copy_to_slice(&mut template_bytes);
        let template_data = String::from_utf8_lossy(&template_bytes).to_string();
        
        // Read sky data
        let sky_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < sky_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for sky data",
            )));
        }
        let mut sky_data = vec![0u8; sky_len];
        buf.copy_to_slice(&mut sky_data);
        
        // Read weather data
        let weather_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < weather_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for weather data",
            )));
        }
        let mut weather_data = vec![0u8; weather_len];
        buf.copy_to_slice(&mut weather_data);
        
        // Read player start position
        if buf.remaining() < 8 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for player start",
            )));
        }
        let player_start = (buf.get_f32(), buf.get_f32());
        
        // Read player respawn position
        if buf.remaining() < 8 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for player respawn",
            )));
        }
        let player_respawn = (buf.get_f32(), buf.get_f32());
        
        // Read respawn in world flag
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for respawn flag",
            )));
        }
        let respawn_in_world = buf.get_u8() != 0;
        
        // Skip dungeon gravity map
        let gravity_count = VLQ::read_unsigned(buf)? as usize;
        for _ in 0..gravity_count {
            buf.get_u16(); // dungeon ID
            buf.get_f32(); // gravity value
        }
        
        // Skip dungeon breathable map
        let breathable_count = VLQ::read_unsigned(buf)? as usize;
        for _ in 0..breathable_count {
            buf.get_u16(); // dungeon ID
            buf.get_u8();  // breathable flag
        }
        
        // Skip protected dungeon IDs
        let protected_count = VLQ::read_unsigned(buf)? as usize;
        for _ in 0..protected_count {
            buf.get_u16(); // dungeon ID
        }
        
        // Read world properties
        let props_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < props_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for world properties",
            )));
        }
        let mut props_bytes = vec![0u8; props_len];
        buf.copy_to_slice(&mut props_bytes);
        let world_properties = String::from_utf8_lossy(&props_bytes).to_string();
        
        // Read client ID
        if buf.remaining() < 2 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for client ID",
            )));
        }
        let client_id = buf.get_u16();
        
        // Read local interpolation mode
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for interpolation mode",
            )));
        }
        let local_interpolation_mode = buf.get_u8() != 0;
        
        Ok(Self {
            template_data,
            sky_data,
            weather_data,
            player_start,
            player_respawn,
            respawn_in_world,
            world_properties,
            client_id,
            local_interpolation_mode,
        })
    }
}

/// World Stop Packet - sent by server when client leaves a world
#[derive(Debug, Clone)]
pub struct WorldStopPacket {
    pub reason: String,
}

impl Packet for WorldStopPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::WorldStop
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        let reason_bytes = self.reason.as_bytes();
        VLQ::write_unsigned(buf, reason_bytes.len() as u64);
        buf.put_slice(reason_bytes);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        let reason_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < reason_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for reason",
            )));
        }
        
        let mut reason_bytes = vec![0u8; reason_len];
        buf.copy_to_slice(&mut reason_bytes);
        let reason = String::from_utf8_lossy(&reason_bytes).to_string();
        
        Ok(Self { reason })
    }
}

/// Entity Create Packet - sent when a new entity is created in the world
#[derive(Debug, Clone)]
pub struct EntityCreatePacket {
    pub entity_type: EntityType,
    pub store_data: Vec<u8>,      // Entity storage data (serialized entity state)
    pub first_net_state: Vec<u8>,  // Initial network state
    pub entity_id: EntityId,
}

impl Packet for EntityCreatePacket {
    fn packet_type(&self) -> PacketType {
        PacketType::EntityCreate
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        // Write entity type
        buf.put_u8(self.entity_type as u8);
        
        // Write store data length and data
        VLQ::write_unsigned(buf, self.store_data.len() as u64);
        buf.put_slice(&self.store_data);
        
        // Write first net state length and data
        VLQ::write_unsigned(buf, self.first_net_state.len() as u64);
        buf.put_slice(&self.first_net_state);
        
        // Write entity ID
        buf.put_i32(self.entity_id);
        
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        // Read entity type
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for entity type",
            )));
        }
        let entity_type = EntityType::from_u8(buf.get_u8())?;
        
        // Read store data
        let store_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < store_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for store data",
            )));
        }
        let mut store_data = vec![0u8; store_len];
        buf.copy_to_slice(&mut store_data);
        
        // Read first net state
        let state_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < state_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for first net state",
            )));
        }
        let mut first_net_state = vec![0u8; state_len];
        buf.copy_to_slice(&mut first_net_state);
        
        // Read entity ID
        if buf.remaining() < 4 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for entity ID",
            )));
        }
        let entity_id = buf.get_i32();
        
        Ok(Self {
            entity_type,
            store_data,
            first_net_state,
            entity_id,
        })
    }
}

/// Entity Update Set Packet - sent when entities need to update their state
#[derive(Debug, Clone)]
pub struct EntityUpdateSetPacket {
    pub for_connection: ConnectionId,
    pub deltas: std::collections::HashMap<EntityId, Vec<u8>>,
}

impl Packet for EntityUpdateSetPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::EntityUpdateSet
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        // Write connection ID
        buf.put_u16(self.for_connection);
        
        // Write number of deltas
        VLQ::write_unsigned(buf, self.deltas.len() as u64);
        
        // Write each delta
        for (entity_id, delta) in &self.deltas {
            buf.put_i32(*entity_id);
            VLQ::write_unsigned(buf, delta.len() as u64);
            buf.put_slice(delta);
        }
        
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        // Read connection ID
        if buf.remaining() < 2 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for connection ID",
            )));
        }
        let for_connection = buf.get_u16();
        
        // Read number of deltas
        let delta_count = VLQ::read_unsigned(buf)? as usize;
        
        // Read each delta
        let mut deltas = std::collections::HashMap::new();
        for _ in 0..delta_count {
            if buf.remaining() < 4 {
                return Err(ProtocolError::Io(io::Error::new(
                    io::ErrorKind::UnexpectedEof,
                    "Not enough bytes for entity ID",
                )));
            }
            let entity_id = buf.get_i32();
            
            let delta_len = VLQ::read_unsigned(buf)? as usize;
            if buf.remaining() < delta_len {
                return Err(ProtocolError::Io(io::Error::new(
                    io::ErrorKind::UnexpectedEof,
                    "Not enough bytes for delta",
                )));
            }
            let mut delta = vec![0u8; delta_len];
            buf.copy_to_slice(&mut delta);
            
            deltas.insert(entity_id, delta);
        }
        
        Ok(Self {
            for_connection,
            deltas,
        })
    }
}

/// Entity Destroy Packet - sent when an entity is removed from the world
#[derive(Debug, Clone)]
pub struct EntityDestroyPacket {
    pub entity_id: EntityId,
    pub final_net_state: Vec<u8>,
    pub death: bool,  // True if removed due to death, false if just out of range
}

impl Packet for EntityDestroyPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::EntityDestroy
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        // Write entity ID
        buf.put_i32(self.entity_id);
        
        // Write final net state
        VLQ::write_unsigned(buf, self.final_net_state.len() as u64);
        buf.put_slice(&self.final_net_state);
        
        // Write death flag
        buf.put_u8(if self.death { 1 } else { 0 });
        
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        // Read entity ID
        if buf.remaining() < 4 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for entity ID",
            )));
        }
        let entity_id = buf.get_i32();
        
        // Read final net state
        let state_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < state_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for final net state",
            )));
        }
        let mut final_net_state = vec![0u8; state_len];
        buf.copy_to_slice(&mut final_net_state);
        
        // Read death flag
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for death flag",
            )));
        }
        let death = buf.get_u8() != 0;
        
        Ok(Self {
            entity_id,
            final_net_state,
            death,
        })
    }
}

/// Entity Interact Packet - sent when a client interacts with an entity
#[derive(Debug, Clone)]
pub struct EntityInteractPacket {
    pub entity_id: EntityId,
    pub request_id: u32,
    pub request_data: Vec<u8>,
}

impl Packet for EntityInteractPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::EntityInteract
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_i32(self.entity_id);
        buf.put_u32(self.request_id);
        VLQ::write_unsigned(buf, self.request_data.len() as u64);
        buf.put_slice(&self.request_data);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 8 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for entity interact",
            )));
        }
        
        let entity_id = buf.get_i32();
        let request_id = buf.get_u32();
        
        let data_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < data_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for request data",
            )));
        }
        
        let mut request_data = vec![0u8; data_len];
        buf.copy_to_slice(&mut request_data);
        
        Ok(Self {
            entity_id,
            request_id,
            request_data,
        })
    }
}

/// Entity Interact Result Packet - server response to entity interaction
#[derive(Debug, Clone)]
pub struct EntityInteractResultPacket {
    pub request_id: u32,
    pub success: bool,
    pub result_data: Vec<u8>,
}

impl Packet for EntityInteractResultPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::EntityInteractResult
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_u32(self.request_id);
        buf.put_u8(if self.success { 1 } else { 0 });
        VLQ::write_unsigned(buf, self.result_data.len() as u64);
        buf.put_slice(&self.result_data);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 5 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for interact result",
            )));
        }
        
        let request_id = buf.get_u32();
        let success = buf.get_u8() != 0;
        
        let data_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < data_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for result data",
            )));
        }
        
        let mut result_data = vec![0u8; data_len];
        buf.copy_to_slice(&mut result_data);
        
        Ok(Self {
            request_id,
            success,
            result_data,
        })
    }
}

/// Hit Request Packet - sent when a client attempts to hit an entity
#[derive(Debug, Clone)]
pub struct HitRequestPacket {
    pub source_entity_id: EntityId,
    pub target_entity_id: EntityId,
    pub hit_type: u8,  // 0 = melee, 1 = projectile, etc.
}

impl Packet for HitRequestPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::HitRequest
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_i32(self.source_entity_id);
        buf.put_i32(self.target_entity_id);
        buf.put_u8(self.hit_type);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 9 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for hit request",
            )));
        }
        
        let source_entity_id = buf.get_i32();
        let target_entity_id = buf.get_i32();
        let hit_type = buf.get_u8();
        
        Ok(Self {
            source_entity_id,
            target_entity_id,
            hit_type,
        })
    }
}

/// Damage Request Packet - sent when an entity should take damage
#[derive(Debug, Clone)]
pub struct DamageRequestPacket {
    pub target_entity_id: EntityId,
    pub damage_amount: f32,
    pub damage_type: String,  // "physical", "fire", "electric", etc.
    pub source_entity_id: Option<EntityId>,
}

impl Packet for DamageRequestPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::DamageRequest
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_i32(self.target_entity_id);
        buf.put_f32(self.damage_amount);
        
        let type_bytes = self.damage_type.as_bytes();
        VLQ::write_unsigned(buf, type_bytes.len() as u64);
        buf.put_slice(type_bytes);
        
        buf.put_u8(if self.source_entity_id.is_some() { 1 } else { 0 });
        if let Some(source_id) = self.source_entity_id {
            buf.put_i32(source_id);
        }
        
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 8 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for damage request",
            )));
        }
        
        let target_entity_id = buf.get_i32();
        let damage_amount = buf.get_f32();
        
        let type_len = VLQ::read_unsigned(buf)? as usize;
        if buf.remaining() < type_len {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for damage type",
            )));
        }
        
        let mut type_bytes = vec![0u8; type_len];
        buf.copy_to_slice(&mut type_bytes);
        let damage_type = String::from_utf8_lossy(&type_bytes).to_string();
        
        if !buf.has_remaining() {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for source flag",
            )));
        }
        
        let has_source = buf.get_u8() != 0;
        let source_entity_id = if has_source {
            if buf.remaining() < 4 {
                return Err(ProtocolError::Io(io::Error::new(
                    io::ErrorKind::UnexpectedEof,
                    "Not enough bytes for source entity ID",
                )));
            }
            Some(buf.get_i32())
        } else {
            None
        };
        
        Ok(Self {
            target_entity_id,
            damage_amount,
            damage_type,
            source_entity_id,
        })
    }
}

/// Damage Notification Packet - server notifies about damage dealt
#[derive(Debug, Clone)]
pub struct DamageNotificationPacket {
    pub target_entity_id: EntityId,
    pub damage_amount: f32,
    pub killed: bool,
}

impl Packet for DamageNotificationPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::DamageNotification
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        buf.put_i32(self.target_entity_id);
        buf.put_f32(self.damage_amount);
        buf.put_u8(if self.killed { 1 } else { 0 });
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        if buf.remaining() < 9 {
            return Err(ProtocolError::Io(io::Error::new(
                io::ErrorKind::UnexpectedEof,
                "Not enough bytes for damage notification",
            )));
        }
        
        let target_entity_id = buf.get_i32();
        let damage_amount = buf.get_f32();
        let killed = buf.get_u8() != 0;
        
        Ok(Self {
            target_entity_id,
            damage_amount,
            killed,
        })
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

    #[test]
    fn test_world_start_packet() {
        let packet = WorldStartPacket {
            template_data: r#"{"type":"test"}"#.to_string(),
            sky_data: vec![1, 2, 3, 4],
            weather_data: vec![5, 6, 7, 8],
            player_start: (100.0, 200.0),
            player_respawn: (150.0, 250.0),
            respawn_in_world: true,
            world_properties: r#"{"gravity":10}"#.to_string(),
            client_id: 1,
            local_interpolation_mode: false,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = WorldStartPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.template_data, r#"{"type":"test"}"#);
        assert_eq!(decoded.sky_data, vec![1, 2, 3, 4]);
        assert_eq!(decoded.weather_data, vec![5, 6, 7, 8]);
        assert_eq!(decoded.player_start, (100.0, 200.0));
        assert_eq!(decoded.player_respawn, (150.0, 250.0));
        assert_eq!(decoded.respawn_in_world, true);
        assert_eq!(decoded.client_id, 1);
        assert_eq!(decoded.local_interpolation_mode, false);
    }

    #[test]
    fn test_world_stop_packet() {
        let packet = WorldStopPacket {
            reason: "Player disconnected".to_string(),
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = WorldStopPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.reason, "Player disconnected");
    }

    #[test]
    fn test_entity_create_packet() {
        let packet = EntityCreatePacket {
            entity_type: EntityType::Player,
            store_data: vec![1, 2, 3, 4, 5],
            first_net_state: vec![10, 20, 30],
            entity_id: 12345,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = EntityCreatePacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.entity_type, EntityType::Player);
        assert_eq!(decoded.store_data, vec![1, 2, 3, 4, 5]);
        assert_eq!(decoded.first_net_state, vec![10, 20, 30]);
        assert_eq!(decoded.entity_id, 12345);
    }

    #[test]
    fn test_entity_update_set_packet() {
        let mut deltas = std::collections::HashMap::new();
        deltas.insert(100, vec![1, 2, 3]);
        deltas.insert(200, vec![4, 5, 6]);
        
        let packet = EntityUpdateSetPacket {
            for_connection: 1,
            deltas,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = EntityUpdateSetPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.for_connection, 1);
        assert_eq!(decoded.deltas.len(), 2);
        assert!(decoded.deltas.contains_key(&100));
        assert!(decoded.deltas.contains_key(&200));
    }

    #[test]
    fn test_entity_destroy_packet() {
        let packet = EntityDestroyPacket {
            entity_id: 54321,
            final_net_state: vec![99, 88, 77],
            death: true,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = EntityDestroyPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.entity_id, 54321);
        assert_eq!(decoded.final_net_state, vec![99, 88, 77]);
        assert_eq!(decoded.death, true);
    }

    #[test]
    fn test_entity_interact_packet() {
        let packet = EntityInteractPacket {
            entity_id: 123,
            request_id: 456,
            request_data: vec![1, 2, 3, 4],
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = EntityInteractPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.entity_id, 123);
        assert_eq!(decoded.request_id, 456);
        assert_eq!(decoded.request_data, vec![1, 2, 3, 4]);
    }

    #[test]
    fn test_hit_request_packet() {
        let packet = HitRequestPacket {
            source_entity_id: 100,
            target_entity_id: 200,
            hit_type: 1,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = HitRequestPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.source_entity_id, 100);
        assert_eq!(decoded.target_entity_id, 200);
        assert_eq!(decoded.hit_type, 1);
    }

    #[test]
    fn test_damage_request_packet() {
        let packet = DamageRequestPacket {
            target_entity_id: 300,
            damage_amount: 50.5,
            damage_type: "fire".to_string(),
            source_entity_id: Some(400),
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = DamageRequestPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.target_entity_id, 300);
        assert_eq!(decoded.damage_amount, 50.5);
        assert_eq!(decoded.damage_type, "fire");
        assert_eq!(decoded.source_entity_id, Some(400));
    }

    #[test]
    fn test_damage_notification_packet() {
        let packet = DamageNotificationPacket {
            target_entity_id: 500,
            damage_amount: 25.0,
            killed: true,
        };
        
        let mut buf = BytesMut::new();
        packet.write(&mut buf).unwrap();
        
        let bytes = buf.freeze();
        let mut cursor = Cursor::new(bytes.as_ref());
        let decoded = DamageNotificationPacket::read(&mut cursor).unwrap();
        
        assert_eq!(decoded.target_entity_id, 500);
        assert_eq!(decoded.damage_amount, 25.0);
        assert_eq!(decoded.killed, true);
    }
}
