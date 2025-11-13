/// Starbound Server Implementation in Rust
/// 
/// This module implements the core server logic that handles client connections,
/// processes packets, and maintains server state.

use crate::protocol::*;
use anyhow::{Context, Result};
use bytes::{Buf, BufMut, BytesMut};
use log::{debug, error, info, warn};
use std::io::Cursor;
use std::net::SocketAddr;
use std::sync::Arc;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::{Mutex, RwLock};
use std::collections::HashMap;

/// Maximum packet size (64 MB) - matches C++ implementation
const MAX_PACKET_SIZE: usize = 64 << 20;

/// Server configuration
#[derive(Debug, Clone)]
pub struct ServerConfig {
    pub bind_address: String,
    pub max_clients: usize,
    pub server_name: String,
}

impl Default for ServerConfig {
    fn default() -> Self {
        Self {
            bind_address: "0.0.0.0:21025".to_string(),
            max_clients: 8,
            server_name: "OpenStarbound Rust Server".to_string(),
        }
    }
}

/// Represents a connected client
struct Client {
    id: u16,
    addr: SocketAddr,
    stream: Arc<Mutex<TcpStream>>,
    authenticated: bool,
    nick: String,
}

/// The main Starbound server
pub struct StarboundServer {
    config: ServerConfig,
    clients: Arc<RwLock<HashMap<u16, Client>>>,
    next_client_id: Arc<Mutex<u16>>,
    running: Arc<RwLock<bool>>,
}

impl StarboundServer {
    pub fn new(config: ServerConfig) -> Self {
        Self {
            config,
            clients: Arc::new(RwLock::new(HashMap::new())),
            next_client_id: Arc::new(Mutex::new(1)),
            running: Arc::new(RwLock::new(false)),
        }
    }

    pub async fn start(&self) -> Result<()> {
        let listener = TcpListener::bind(&self.config.bind_address)
            .await
            .context("Failed to bind to address")?;
        
        info!("Starbound Rust Server listening on {}", self.config.bind_address);
        info!("Protocol Version: {}", PROTOCOL_VERSION);
        info!("Server Name: {}", self.config.server_name);
        
        *self.running.write().await = true;

        loop {
            if !*self.running.read().await {
                break;
            }

            match listener.accept().await {
                Ok((stream, addr)) => {
                    info!("New connection from {}", addr);
                    
                    let clients = self.clients.clone();
                    let next_id = self.next_client_id.clone();
                    let config = self.config.clone();
                    
                    tokio::spawn(async move {
                        if let Err(e) = Self::handle_client(stream, addr, clients, next_id, config).await {
                            error!("Error handling client {}: {}", addr, e);
                        }
                    });
                }
                Err(e) => {
                    error!("Failed to accept connection: {}", e);
                }
            }
        }

        Ok(())
    }

    pub async fn stop(&self) {
        info!("Stopping server...");
        *self.running.write().await = false;
    }

    async fn handle_client(
        mut stream: TcpStream,
        addr: SocketAddr,
        clients: Arc<RwLock<HashMap<u16, Client>>>,
        next_id: Arc<Mutex<u16>>,
        config: ServerConfig,
    ) -> Result<()> {
        // Perform handshake
        let client_id = match Self::perform_handshake(&mut stream, &config).await {
            Ok(id) => id,
            Err(e) => {
                warn!("Handshake failed for {}: {}", addr, e);
                return Err(e);
            }
        };

        info!("Client {} connected as ID {}", addr, client_id);

        let stream_arc = Arc::new(Mutex::new(stream));
        
        // Register client
        {
            let mut clients_lock = clients.write().await;
            clients_lock.insert(
                client_id,
                Client {
                    id: client_id,
                    addr,
                    stream: stream_arc.clone(),
                    authenticated: false,
                    nick: format!("Player{}", client_id),
                },
            );
        }

        // Send server info packet
        {
            let client_count = clients.read().await.len() as u16;
            let server_info = ServerInfoPacket {
                players: client_count,
                max_players: config.max_clients as u16,
            };
            let mut stream_lock = stream_arc.lock().await;
            let _ = Self::write_packet(&mut *stream_lock, &server_info).await;
        }

        // Main client loop - handle incoming packets
        loop {
            let packet_result = {
                let mut stream_lock = stream_arc.lock().await;
                Self::read_packet_type(&mut *stream_lock).await
            };

            match packet_result {
                Ok((packet_type, packet_data)) => {
                    match packet_type {
                        PacketType::ChatSend => {
                            if let Err(e) = Self::handle_chat_send(
                                client_id,
                                &packet_data,
                                &clients,
                                &config,
                            ).await {
                                error!("Error handling chat send: {}", e);
                            }
                        }
                        PacketType::ClientDisconnectRequest => {
                            info!("Client {} requested disconnect", client_id);
                            break;
                        }
                        _ => {
                            debug!("Unhandled packet type: {:?}", packet_type);
                        }
                    }
                }
                Err(e) => {
                    warn!("Error reading packet from client {}: {}", client_id, e);
                    break;
                }
            }
        }

        // Cleanup
        {
            let mut clients_lock = clients.write().await;
            clients_lock.remove(&client_id);
        }
        info!("Client {} (ID {}) disconnected", addr, client_id);

        Ok(())
    }

    async fn perform_handshake(stream: &mut TcpStream, config: &ServerConfig) -> Result<u16> {
        // Read ProtocolRequest packet
        let protocol_request = Self::read_packet::<ProtocolRequestPacket>(stream).await?;
        
        debug!("Received protocol request: version {}", protocol_request.request_protocol_version);

        // Check protocol version
        if protocol_request.request_protocol_version != PROTOCOL_VERSION {
            // Send rejection response
            let response = ProtocolResponsePacket {
                allowed: false,
                info: serde_json::json!({
                    "error": "Protocol version mismatch",
                    "server_version": PROTOCOL_VERSION,
                    "client_version": protocol_request.request_protocol_version
                }).to_string(),
            };
            Self::write_packet(stream, &response).await?;
            
            return Err(anyhow::anyhow!(
                "Protocol version mismatch: expected {}, got {}",
                PROTOCOL_VERSION,
                protocol_request.request_protocol_version
            ));
        }

        // Send acceptance response
        let response = ProtocolResponsePacket {
            allowed: true,
            info: serde_json::json!({
                "server_name": config.server_name,
                "version": PROTOCOL_VERSION,
            }).to_string(),
        };
        Self::write_packet(stream, &response).await?;
        
        debug!("Protocol handshake successful");

        // For MVP, we skip the full handshake and just return a client ID
        // Full implementation would handle HandshakeChallenge and ClientConnect
        Ok(1) // Simplified client ID assignment
    }

    async fn read_packet<P: Packet>(stream: &mut TcpStream) -> Result<P> {
        // Read packet type (1 byte)
        let packet_type = stream.read_u8().await?;
        let packet_type = PacketType::from_u8(packet_type)?;
        
        debug!("Reading packet type: {:?}", packet_type);

        // Read packet size (VLQ signed integer)
        let size = Self::read_vlq_signed(stream).await?;
        let (actual_size, compressed) = if size < 0 {
            ((-size) as usize, true)
        } else {
            (size as usize, false)
        };

        if actual_size > MAX_PACKET_SIZE {
            return Err(ProtocolError::PacketTooLarge(actual_size).into());
        }

        debug!("Packet size: {} bytes (compressed: {})", actual_size, compressed);

        // Read packet data
        let mut packet_data = vec![0u8; actual_size];
        stream.read_exact(&mut packet_data).await?;

        // For MVP, we don't handle compression yet
        if compressed {
            warn!("Compressed packets not yet supported in MVP");
            return Err(anyhow::anyhow!("Compressed packets not supported"));
        }

        // Parse packet
        let mut cursor = Cursor::new(packet_data.as_slice());
        let packet = P::read(&mut cursor)?;

        Ok(packet)
    }

    async fn write_packet<P: Packet>(stream: &mut TcpStream, packet: &P) -> Result<()> {
        let mut buf = BytesMut::new();
        
        // Write packet type
        buf.put_u8(packet.packet_type() as u8);
        
        // Serialize packet data
        let mut packet_buf = BytesMut::new();
        packet.write(&mut packet_buf)?;
        
        // Write packet size (VLQ signed integer, positive = uncompressed)
        let size = packet_buf.len() as i64;
        Self::write_vlq_signed(&mut buf, size);
        
        // Write packet data
        buf.put_slice(&packet_buf);
        
        // Send to stream
        stream.write_all(&buf).await?;
        stream.flush().await?;
        
        debug!("Sent packet type {:?}, size {} bytes", packet.packet_type(), packet_buf.len());
        
        Ok(())
    }

    async fn read_packet_type(stream: &mut TcpStream) -> Result<(PacketType, Vec<u8>)> {
        // Read packet type (1 byte)
        let packet_type = stream.read_u8().await?;
        let packet_type = PacketType::from_u8(packet_type)?;
        
        debug!("Reading packet type: {:?}", packet_type);

        // Read packet size (VLQ signed integer)
        let size = Self::read_vlq_signed(stream).await?;
        let (actual_size, compressed) = if size < 0 {
            ((-size) as usize, true)
        } else {
            (size as usize, false)
        };

        if actual_size > MAX_PACKET_SIZE {
            return Err(ProtocolError::PacketTooLarge(actual_size).into());
        }

        debug!("Packet size: {} bytes (compressed: {})", actual_size, compressed);

        // Read packet data
        let mut packet_data = vec![0u8; actual_size];
        stream.read_exact(&mut packet_data).await?;

        // For MVP, we don't handle compression yet
        if compressed {
            warn!("Compressed packets not yet supported in MVP");
            return Err(anyhow::anyhow!("Compressed packets not supported"));
        }

        Ok((packet_type, packet_data))
    }

    async fn handle_chat_send(
        client_id: u16,
        packet_data: &[u8],
        clients: &Arc<RwLock<HashMap<u16, Client>>>,
        config: &ServerConfig,
    ) -> Result<()> {
        // Parse the chat send packet
        let mut cursor = Cursor::new(packet_data);
        let chat_packet = ChatSendPacket::read(&mut cursor)?;
        
        info!("Chat from client {}: {}", client_id, chat_packet.text);

        // Check for admin commands
        if chat_packet.text.starts_with('/') {
            Self::handle_admin_command(client_id, &chat_packet.text, clients, config).await?;
            return Ok(());
        }

        // Get sender nick
        let sender_nick = {
            let clients_lock = clients.read().await;
            clients_lock.get(&client_id)
                .map(|c| c.nick.clone())
                .unwrap_or_else(|| format!("Player{}", client_id))
        };

        // Create chat receive message
        let chat_receive = ChatReceivePacket {
            received_message: ChatReceivedMessage {
                context: MessageContext::new(match chat_packet.send_mode {
                    ChatSendMode::Broadcast => MessageContextMode::Broadcast,
                    ChatSendMode::Local => MessageContextMode::Local,
                    ChatSendMode::Party => MessageContextMode::Party,
                }),
                from_connection: client_id,
                from_nick: sender_nick,
                portrait: String::new(),
                text: chat_packet.text,
            },
        };

        // Broadcast to all clients
        Self::broadcast_packet(clients, &chat_receive).await?;

        Ok(())
    }

    async fn handle_admin_command(
        client_id: u16,
        command: &str,
        clients: &Arc<RwLock<HashMap<u16, Client>>>,
        config: &ServerConfig,
    ) -> Result<()> {
        let parts: Vec<&str> = command.split_whitespace().collect();
        if parts.is_empty() {
            return Ok(());
        }

        let cmd = parts[0].to_lowercase();
        let response_text = match cmd.as_str() {
            "/help" => {
                "Available commands:\n\
                 /help - Show this help\n\
                 /players - List connected players\n\
                 /nick <name> - Change your nickname\n\
                 /broadcast <message> - Broadcast a message (alias: /bc)\n\
                 /info - Show server information"
            }
            "/players" => {
                let clients_lock = clients.read().await;
                let player_list: Vec<String> = clients_lock.values()
                    .map(|c| format!("{} (ID: {})", c.nick, c.id))
                    .collect();
                return Self::send_command_result(
                    client_id,
                    &format!("Connected players ({}):\n{}", 
                        clients_lock.len(), 
                        player_list.join("\n")),
                    clients,
                ).await;
            }
            "/nick" if parts.len() > 1 => {
                let new_nick = parts[1..].join(" ");
                {
                    let mut clients_lock = clients.write().await;
                    if let Some(client) = clients_lock.get_mut(&client_id) {
                        let old_nick = client.nick.clone();
                        client.nick = new_nick.clone();
                        return Self::broadcast_system_message(
                            &format!("{} is now known as {}", old_nick, new_nick),
                            clients,
                        ).await;
                    }
                }
                "Failed to change nickname"
            }
            "/broadcast" | "/bc" if parts.len() > 1 => {
                let message = parts[1..].join(" ");
                return Self::broadcast_system_message(&message, clients).await;
            }
            "/info" => {
                let clients_lock = clients.read().await;
                return Self::send_command_result(
                    client_id,
                    &format!(
                        "Server: {}\nPlayers: {}/{}\nProtocol Version: {}",
                        config.server_name,
                        clients_lock.len(),
                        config.max_clients,
                        PROTOCOL_VERSION
                    ),
                    clients,
                ).await;
            }
            _ => "Unknown command. Type /help for available commands."
        };

        Self::send_command_result(client_id, response_text, clients).await
    }

    async fn send_command_result(
        client_id: u16,
        message: &str,
        clients: &Arc<RwLock<HashMap<u16, Client>>>,
    ) -> Result<()> {
        let packet = ChatReceivePacket {
            received_message: ChatReceivedMessage {
                context: MessageContext::new(MessageContextMode::CommandResult),
                from_connection: 0,
                from_nick: "Server".to_string(),
                portrait: String::new(),
                text: message.to_string(),
            },
        };

        // Send only to the requesting client
        let clients_lock = clients.read().await;
        if let Some(client) = clients_lock.get(&client_id) {
            let mut stream_lock = client.stream.lock().await;
            Self::write_packet(&mut *stream_lock, &packet).await?;
        }

        Ok(())
    }

    async fn broadcast_system_message(
        message: &str,
        clients: &Arc<RwLock<HashMap<u16, Client>>>,
    ) -> Result<()> {
        let packet = ChatReceivePacket {
            received_message: ChatReceivedMessage {
                context: MessageContext::new(MessageContextMode::Broadcast),
                from_connection: 0,
                from_nick: "Server".to_string(),
                portrait: String::new(),
                text: message.to_string(),
            },
        };

        Self::broadcast_packet(clients, &packet).await
    }

    async fn broadcast_packet<P: Packet>(
        clients: &Arc<RwLock<HashMap<u16, Client>>>,
        packet: &P,
    ) -> Result<()> {
        let clients_lock = clients.read().await;
        
        for client in clients_lock.values() {
            let mut stream_lock = client.stream.lock().await;
            if let Err(e) = Self::write_packet(&mut *stream_lock, packet).await {
                warn!("Failed to send packet to client {}: {}", client.id, e);
            }
        }

        Ok(())
    }

    async fn read_vlq_signed(stream: &mut TcpStream) -> Result<i64> {
        let mut result: u64 = 0;
        let mut shift = 0;
        
        loop {
            let byte = stream.read_u8().await?;
            result |= ((byte & 0x7F) as u64) << shift;
            
            if (byte & 0x80) == 0 {
                break;
            }
            
            shift += 7;
            if shift >= 64 {
                return Err(anyhow::anyhow!("VLQ overflow"));
            }
        }
        
        // Zigzag decode
        let signed_result = ((result >> 1) as i64) ^ (-((result & 1) as i64));
        Ok(signed_result)
    }

    fn write_vlq_signed(buf: &mut BytesMut, value: i64) {
        // Zigzag encode
        let unsigned = ((value << 1) ^ (value >> 63)) as u64;
        Self::write_vlq_unsigned(buf, unsigned);
    }

    fn write_vlq_unsigned(buf: &mut BytesMut, mut value: u64) {
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
}
