/// Simple example client demonstrating protocol handshake
/// This shows how the protocol works from the client side
/// 
/// Run the server first, then run this with:
/// cargo run --example simple_client

use bytes::{BufMut, BytesMut};
use std::io::Cursor;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;

const PROTOCOL_VERSION: u32 = 747;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("OpenStarbound Rust Client Example");
    println!("==================================");
    
    // Connect to server
    let server_addr = std::env::var("SERVER_ADDR").unwrap_or_else(|_| "127.0.0.1:21025".to_string());
    println!("Connecting to {}...", server_addr);
    
    let mut stream = TcpStream::connect(&server_addr).await?;
    println!("Connected!");
    
    // Send ProtocolRequest
    println!("\nSending ProtocolRequest (version {})...", PROTOCOL_VERSION);
    send_protocol_request(&mut stream).await?;
    
    // Receive ProtocolResponse
    println!("Waiting for ProtocolResponse...");
    let response = receive_protocol_response(&mut stream).await?;
    
    if response.allowed {
        println!("✓ Connection ACCEPTED!");
        println!("  Server info: {}", response.info);
    } else {
        println!("✗ Connection REJECTED");
        println!("  Reason: {}", response.info);
    }
    
    println!("\nHandshake complete!");
    
    Ok(())
}

async fn send_protocol_request(stream: &mut TcpStream) -> Result<(), Box<dyn std::error::Error>> {
    let mut buf = BytesMut::new();
    
    // Packet type (ProtocolRequest = 0)
    buf.put_u8(0);
    
    // Packet size (4 bytes for u32)
    write_vlq_signed(&mut buf, 4);
    
    // Protocol version
    buf.put_u32(PROTOCOL_VERSION);
    
    stream.write_all(&buf).await?;
    stream.flush().await?;
    
    Ok(())
}

#[derive(Debug)]
struct ProtocolResponse {
    allowed: bool,
    info: String,
}

async fn receive_protocol_response(stream: &mut TcpStream) -> Result<ProtocolResponse, Box<dyn std::error::Error>> {
    // Read packet type
    let packet_type = stream.read_u8().await?;
    if packet_type != 1 {
        return Err(format!("Expected ProtocolResponse (1), got {}", packet_type).into());
    }
    
    // Read packet size
    let size = read_vlq_signed(stream).await?;
    let actual_size = if size < 0 { (-size) as usize } else { size as usize };
    
    // Read packet data
    let mut packet_data = vec![0u8; actual_size];
    stream.read_exact(&mut packet_data).await?;
    
    // Parse packet
    let mut cursor = Cursor::new(packet_data.as_slice());
    
    // Read allowed flag
    let mut allowed_byte = [0u8; 1];
    std::io::Read::read_exact(&mut cursor, &mut allowed_byte)?;
    let allowed = allowed_byte[0] != 0;
    
    // Read info string
    let info_len = read_vlq_unsigned(&mut cursor)?;
    let mut info_bytes = vec![0u8; info_len as usize];
    std::io::Read::read_exact(&mut cursor, &mut info_bytes)?;
    let info = String::from_utf8_lossy(&info_bytes).to_string();
    
    Ok(ProtocolResponse { allowed, info })
}

fn write_vlq_signed(buf: &mut BytesMut, value: i64) {
    let unsigned = ((value << 1) ^ (value >> 63)) as u64;
    write_vlq_unsigned(buf, unsigned);
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

async fn read_vlq_signed(stream: &mut TcpStream) -> Result<i64, Box<dyn std::error::Error>> {
    let unsigned = read_vlq_unsigned_stream(stream).await?;
    let signed = ((unsigned >> 1) as i64) ^ (-((unsigned & 1) as i64));
    Ok(signed)
}

fn read_vlq_unsigned(cursor: &mut Cursor<&[u8]>) -> Result<u64, Box<dyn std::error::Error>> {
    let mut result: u64 = 0;
    let mut shift = 0;
    
    loop {
        let mut byte_buf = [0u8; 1];
        std::io::Read::read_exact(cursor, &mut byte_buf)?;
        let byte = byte_buf[0];
        
        result |= ((byte & 0x7F) as u64) << shift;
        
        if (byte & 0x80) == 0 {
            break;
        }
        
        shift += 7;
        if shift >= 64 {
            return Err("VLQ overflow".into());
        }
    }
    
    Ok(result)
}

async fn read_vlq_unsigned_stream(stream: &mut TcpStream) -> Result<u64, Box<dyn std::error::Error>> {
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
            return Err("VLQ overflow".into());
        }
    }
    
    Ok(result)
}
