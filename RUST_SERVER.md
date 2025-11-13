# Rust Server Implementation for OpenStarbound

## Overview

This document describes the Rust server implementation for OpenStarbound. The implementation provides a foundation for running a Starbound server written in Rust while maintaining full binary protocol compatibility with the existing C++ client.

## Motivation

The Rust implementation offers several potential benefits:
- **Memory Safety**: Rust's ownership system prevents many common bugs
- **Concurrency**: Modern async/await pattern for efficient connection handling
- **Performance**: Comparable to C++ with better safety guarantees
- **Maintainability**: More expressive type system and error handling

## Architecture

### Project Structure

```
rust_server/
├── Cargo.toml          # Rust project configuration and dependencies
├── README.md           # Rust server documentation
└── src/
    ├── main.rs         # Entry point and server initialization
    ├── protocol.rs     # Network protocol implementation
    └── server.rs       # Server logic and connection handling
```

### Key Components

#### 1. Protocol Layer (`protocol.rs`)

Implements the Starbound network protocol with exact binary compatibility:

- **Protocol Version**: 747 (matches C++ implementation)
- **VLQ Encoding**: Variable Length Quantity encoding for efficient integer serialization
- **Packet Types**: Enum of all packet types (handshake, game, world, etc.)
- **Packet Serialization**: Read/write methods for binary protocol

**Key Types:**
- `PacketType`: Enum representing all packet types
- `Packet` trait: Interface for packet serialization/deserialization
- `VLQ`: Variable Length Quantity encoder/decoder
- `ProtocolRequestPacket`, `ProtocolResponsePacket`, etc.: Concrete packet implementations

#### 2. Server Layer (`server.rs`)

Handles network connections and client management:

- **TCP Server**: Async TCP listener for incoming connections
- **Connection Handling**: Per-client connection management
- **Handshake**: Protocol version negotiation with clients
- **Packet Processing**: Reading and writing packets over TCP

**Key Types:**
- `StarboundServer`: Main server managing all clients
- `ServerConfig`: Server configuration (bind address, max clients, etc.)
- `Client`: Represents a connected client

#### 3. Main Entry Point (`main.rs`)

Server initialization and configuration:

- Environment variable configuration
- Logging setup
- Signal handling (Ctrl+C graceful shutdown)
- Server lifecycle management

## Protocol Compatibility

### Wire Format

The implementation maintains exact binary compatibility with the C++ server. Each packet follows this format:

```
[PacketType: u8][Size: VLQ i64][Data: bytes]
```

- **PacketType**: 1 byte identifying the packet type
- **Size**: Variable-length signed integer (VLQ)
  - Positive values: uncompressed packet size
  - Negative values: compressed packet size (absolute value)
- **Data**: Packet payload bytes

### VLQ Encoding

Variable Length Quantity encoding matches the C++ implementation:

- Each byte uses 7 bits for data, 1 bit for continuation
- Continuation bit (0x80) indicates more bytes follow
- Signed integers use zigzag encoding: `(n << 1) ^ (n >> 63)`
- Small numbers use fewer bytes (1-10 bytes for any 64-bit integer)

Example:
```
Value 0:     0x00         (1 byte)
Value 127:   0x7F         (1 byte)
Value 128:   0x80 0x01    (2 bytes)
Value -1:    0x01         (1 byte, zigzag encoded)
```

### Handshake Sequence

The protocol handshake matches the C++ implementation:

1. **Client → Server**: `ProtocolRequestPacket`
   - Contains client protocol version (must be 747)

2. **Server → Client**: `ProtocolResponsePacket`
   - `allowed: bool` - true if version matches
   - `info: String` - JSON with server info or error details

3. If `allowed` is false, connection is rejected

## Current Implementation Status

### ✅ Implemented Features

- **TCP Server**: Async TCP server using Tokio
- **Protocol Handshake**: Full protocol version negotiation
- **VLQ Encoding/Decoding**: Complete implementation with tests
- **Basic Packets**: ProtocolRequest, ProtocolResponse
- **Client Management**: Connection tracking and lifecycle
- **Configuration**: Environment variable configuration
- **Logging**: Structured logging with env_logger
- **Graceful Shutdown**: Ctrl+C handling

### ⬜ Not Yet Implemented (Future Work)

This MVP implementation focuses on protocol compatibility. Full game functionality requires:

- **Packet Compression**: Zstd compression/decompression
- **All Packet Types**: ~50+ packet types for full game support
- **World Management**: Loading, saving, and simulating worlds
- **Entity System**: Players, NPCs, monsters, objects
- **Physics**: Collision detection and movement
- **Celestial Database**: Universe/planet generation
- **Chat System**: Message handling and broadcast
- **Authentication**: Player account validation
- **Persistence**: Save/load player and world data
- **Admin Commands**: Server administration
- **Query Protocol**: Server browser integration
- **RCON**: Remote console access

## Building and Running

### Prerequisites

- Rust 1.70 or later
- Cargo (comes with Rust)

### Build

```bash
cd rust_server

# Debug build
cargo build

# Release build (optimized)
cargo build --release
```

### Run

```bash
# Run with default configuration
cargo run

# Run with custom configuration
SERVER_BIND="0.0.0.0:21025" \
SERVER_NAME="My Rust Server" \
MAX_CLIENTS="16" \
cargo run --release
```

### Test

```bash
# Run all tests
cargo test

# Run with output
cargo test -- --nocapture
```

## Configuration

Environment variables:

| Variable | Default | Description |
|----------|---------|-------------|
| `SERVER_BIND` | `0.0.0.0:21025` | Server bind address and port |
| `SERVER_NAME` | `OpenStarbound Rust Server` | Server name shown to clients |
| `MAX_CLIENTS` | `8` | Maximum concurrent clients |
| `RUST_LOG` | `info` | Log level (error, warn, info, debug, trace) |

## Testing Protocol Compatibility

To verify protocol compatibility with the C++ client:

1. Build the Rust server:
   ```bash
   cd rust_server
   cargo build --release
   ```

2. Run the Rust server:
   ```bash
   cargo run --release
   ```

3. Attempt to connect with the C++ client
   - The client should successfully complete the protocol handshake
   - Connection will fail after handshake (expected - game logic not implemented)
   - But the handshake success proves protocol compatibility

## Development Guidelines

### Adding New Packet Types

1. Add packet type to `PacketType` enum in `protocol.rs`
2. Implement packet struct with fields matching C++ implementation
3. Implement `Packet` trait with `read()` and `write()` methods
4. Add tests verifying serialization/deserialization
5. Handle packet in server logic (`server.rs`)

Example:
```rust
#[derive(Debug, Clone)]
pub struct ChatSendPacket {
    pub message: String,
    pub send_mode: u8,
}

impl Packet for ChatSendPacket {
    fn packet_type(&self) -> PacketType {
        PacketType::ChatSend
    }
    
    fn write(&self, buf: &mut BytesMut) -> Result<(), ProtocolError> {
        // Write fields in exact order as C++
        let msg_bytes = self.message.as_bytes();
        VLQ::write_unsigned(buf, msg_bytes.len() as u64);
        buf.put_slice(msg_bytes);
        buf.put_u8(self.send_mode);
        Ok(())
    }
    
    fn read(buf: &mut Cursor<&[u8]>) -> Result<Self, ProtocolError> {
        // Read fields in exact order as C++
        let msg_len = VLQ::read_unsigned(buf)? as usize;
        let mut msg_bytes = vec![0u8; msg_len];
        buf.copy_to_slice(&mut msg_bytes);
        let message = String::from_utf8_lossy(&msg_bytes).to_string();
        let send_mode = buf.get_u8();
        Ok(Self { message, send_mode })
    }
}
```

### Maintaining Protocol Compatibility

When implementing new features:

1. **Match C++ byte order**: Use big-endian (network byte order) where C++ does
2. **Match field order**: Write/read fields in exact same order as C++
3. **Match encoding**: Use VLQ for sizes, lengths as in C++
4. **Test binary compatibility**: Verify serialized bytes match C++
5. **Version carefully**: Increment protocol version if breaking changes needed

## Performance Considerations

The Rust implementation uses modern async I/O:

- **Tokio Runtime**: Efficient async task scheduling
- **Zero-Copy**: BytesMut for efficient buffer management
- **Connection Pooling**: Async per-client handlers
- **Backpressure**: Tokio's flow control prevents overload

Expected performance comparable to C++ with better safety.

## Security

Rust provides memory safety guarantees:

- **No Buffer Overflows**: Bounds checking on all array access
- **No Use-After-Free**: Ownership system prevents dangling pointers
- **No Data Races**: Borrow checker enforces safe concurrency
- **No Null Pointer Derefs**: `Option<T>` instead of nullable pointers

Additional security features to implement:
- Rate limiting per client
- Packet size validation (MAX_PACKET_SIZE enforced)
- Input sanitization
- Authentication and authorization

## Migration Path

Suggested approach for full migration:

1. **Phase 1 (Current)**: Protocol compatibility and handshake ✅
2. **Phase 2**: Basic packet types (chat, admin commands)
3. **Phase 3**: World loading and basic entity support
4. **Phase 4**: Full entity system and physics
5. **Phase 5**: Persistence and state management
6. **Phase 6**: Feature parity with C++ server
7. **Phase 7**: Rust-specific optimizations and features

Each phase maintains backward compatibility with C++ clients.

## Troubleshooting

### Build Errors

If you encounter build errors:

```bash
# Update Rust
rustup update

# Clean and rebuild
cargo clean
cargo build
```

### Connection Issues

If clients can't connect:

1. Check firewall allows port 21025
2. Verify bind address is correct
3. Check logs: `RUST_LOG=debug cargo run`
4. Ensure protocol version matches (747)

### Protocol Errors

If handshake fails:

1. Verify client protocol version is 747
2. Check packet serialization with tests
3. Enable trace logging: `RUST_LOG=trace cargo run`
4. Compare wire format with C++ using packet captures

## Future Enhancements

Potential Rust-specific improvements:

- **WebSocket Support**: Alternative to TCP for web clients
- **Metrics**: Prometheus-compatible server metrics
- **Hot Reload**: Dynamic configuration updates
- **Distributed Mode**: Multiple server instances
- **Plugin System**: Safe Rust-based mod API
- **Better Observability**: Structured logging and tracing

## Contributing

When contributing to the Rust server:

1. Maintain protocol compatibility
2. Add tests for new features
3. Update documentation
4. Follow Rust conventions (`cargo fmt`, `cargo clippy`)
5. Keep dependencies minimal

## References

- OpenStarbound C++ source: `/source`
- Protocol definition: `/source/game/StarNetPackets.hpp`
- Original server: `/source/game/StarUniverseServer.cpp`
- Rust server: `/rust_server`

## License

Same license as OpenStarbound project.
