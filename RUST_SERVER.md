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

### ✅ Implemented Features (Phase 1, 2, 3, 4, 5 & 6)

**Phase 1: Protocol Foundation** ✅
- **TCP Server**: Async TCP server using Tokio
- **Protocol Handshake**: Full protocol version negotiation
- **VLQ Encoding/Decoding**: Complete implementation with tests
- **Basic Packets**: ProtocolRequest, ProtocolResponse, ServerDisconnect, ConnectSuccess
- **Client Management**: Connection tracking and lifecycle
- **Configuration**: Environment variable configuration
- **Logging**: Structured logging with env_logger
- **Graceful Shutdown**: Ctrl+C handling

**Phase 2: Chat & Admin Commands** ✅
- **Chat System**: Full message handling and broadcasting
  - ChatSendPacket (client → server)
  - ChatReceivePacket (server → client)
  - Multiple chat modes: Broadcast, Local, Party
  - Message context types: Whisper, CommandResult, RadioMessage, World
- **Admin Commands**: Server administration via chat
  - `/help` - Show available commands
  - `/players` - List connected players with IDs
  - `/nick <name>` - Change player nickname
  - `/broadcast <msg>` - Server-wide broadcast (alias: `/bc`)
  - `/info` - Display server information (name, player count, protocol version)
- **ServerInfoPacket**: Server status updates (player count, max players)
- **Nickname Management**: Players can change their display names
- **System Messages**: Server announcements and command results

**Phase 3: Compression & World Packets** ✅
- **Packet Compression**: Zstd compression/decompression
  - Automatic compression for packets > 64 bytes
  - Compression/decompression with size limits
  - Binary compatible with C++ implementation
  - Compress/decompress helper functions with tests
- **World Packet Types**: Infrastructure for world management
  - WorldStartPacket: Template data, sky/weather data, spawn positions, world properties
  - WorldStopPacket: Clean world disconnect with reason
  - Full packet serialization matching C++ wire format
- **Compression Tests**: Round-trip and size reduction validation

**Phase 4: Entity System Foundation** ✅
- **Entity Types**: Complete EntityType enum
  - Plant, Object, Vehicle, ItemDrop, PlantDrop, Projectile
  - Stagehand, Monster, Npc, Player
  - Binary-compatible with C++ EntityType enum
- **Core Entity Packets**: Infrastructure for entity management
  - EntityCreatePacket: Spawn entities with type, store data, initial state
  - EntityUpdateSetPacket: Batch entity state updates with deltas
  - EntityDestroyPacket: Remove entities with final state and death flag
  - All packets binary-compatible with C++ implementation
- **Entity Tests**: Serialization tests for all entity packets

**Phase 5: World & Entity Integration** ✅
- **World Structure**: Complete world representation
  - World ID, template data, sky/weather data
  - Spawn positions and properties
  - Tick counter for simulation
- **EntityManager**: Entity tracking per world
  - Entity storage with HashMap
  - Entity ID allocation
  - Add/remove entity operations
  - Entity lifecycle management
- **WorldManager**: Multi-world management
  - Async world operations with RwLock
  - World creation and removal
  - World lookup and enumeration
- **Integration Infrastructure**: World-entity coordination
  - World tick simulation framework
  - Entity update generation
  - WorldStart packet generation from world state
- **Tests**: World creation, entity management, world manager tests

**Phase 6: Entity Behavior & Player State** ✅
- **World Metadata**: World file format structure
  - WorldMetadata with name, size, spawn point
  - Gravity, breathable atmosphere, biome
  - JSON parsing support (simplified for MVP)
- **Entity Behavior System**: Trait-based behavior framework
  - EntityBehavior trait with update and should_remove methods
  - StaticBehavior for non-moving entities
  - ProjectileBehavior with velocity and lifetime
  - PlayerBehavior for client-controlled entities
- **Player State Management**: Complete player state tracking
  - PlayerState with health, max_health, energy, max_energy
  - Position and velocity tracking
  - Facing direction
  - Damage and healing methods
  - Position/velocity updates
- **Tests**: Metadata, player state, and behavior tests

**Phase 7: World Files & Advanced Interactions** ✅
- **World File I/O**: World loading/saving infrastructure
  - Async file operations with Tokio
  - JSON metadata format
  - World save/load from disk
- **Additional Entity Packets**: Interaction and combat packets
  - EntityInteractPacket (client → server)
  - EntityInteractResultPacket (server → client)
  - HitRequestPacket (combat initiation)
  - DamageRequestPacket (damage application)
  - DamageNotificationPacket (damage confirmation)
- **Collision Detection System**: Basic AABB collision
  - CollisionBox structure
  - CollisionSystem for entity collision tracking
  - Box-box intersection tests
  - Point-in-box tests
- **Tests**: World I/O, collision detection, interaction packets (7 new tests, 29 total)

### ⬜ Not Yet Implemented (Future Work)

**Phase 8: Advanced AI & Pathfinding**
- **Advanced Entity AI**: Behavior trees, decision systems
- **Pathfinding**: A* pathfinding for NPCs and monsters
- **Entity behaviors**: Monster AI, NPC behaviors

**Phase 8: Full Feature Parity**
- **Celestial Database**: Universe/planet generation
- **Authentication**: Player account validation
- **Persistence**: Save/load player and world data
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

1. **Phase 1 (Complete)**: Protocol compatibility and handshake ✅
   - TCP server with async I/O
   - Protocol version negotiation
   - VLQ encoding/decoding
   - Basic packet infrastructure

2. **Phase 2 (Complete)**: Basic packet types (chat, admin commands) ✅
   - Chat message sending and receiving
   - Message broadcasting to all clients
   - Admin command system (/help, /players, /nick, /broadcast, /info)
   - ServerInfoPacket for status updates
   - Player nickname management

3. **Phase 3 (Complete)**: Compression and World Packets ✅
   - Zstd packet compression/decompression
   - Automatic compression for large packets (> 64 bytes)
   - WorldStart/WorldStop packet types
   - World initialization infrastructure
   - Binary protocol compatibility for compression

4. **Phase 4 (Complete)**: Entity System Foundation ✅
   - EntityType enum (10 entity types)
   - EntityCreatePacket (entity spawning)
   - EntityUpdateSetPacket (batch state updates)
   - EntityDestroyPacket (entity removal)
   - Binary protocol compatibility for all entity packets

5. **Phase 5 (Complete)**: World & Entity Integration ✅
   - World structure and management
   - EntityManager for per-world entity tracking
   - WorldManager for multi-world support
   - World simulation tick infrastructure
   - Entity lifecycle management
   - Async operations with RwLock

6. **Phase 6 (Complete)**: Entity Behavior & Player State ✅
   - WorldMetadata structure (world file format)
   - EntityBehavior trait system
   - StaticBehavior, ProjectileBehavior, PlayerBehavior
   - PlayerState with health, energy, position, velocity
   - Damage, healing, and state update methods

7. **Phase 7 (Complete)**: World Files & Advanced Interactions ✅
   - World file I/O from disk (load/save)
   - Additional entity packets (Interact, InteractResult, Hit, Damage, DamageNotification)
   - Collision detection system (AABB)
   - 29 tests total (7 new)

8. **Phase 8 (Next)**: Advanced AI & Pathfinding
   - Advanced entity AI (behavior trees)
   - A* pathfinding for NPCs
   - Monster and NPC behavior systems

9. **Phase 9**: Persistence and Full Features
   - Player save/load
   - World persistence
   - Universe coordination
   - Feature parity with C++ server

9. **Phase 9**: Rust-specific optimizations
   - Performance tuning
   - Advanced async patterns
   - Memory optimizations

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
