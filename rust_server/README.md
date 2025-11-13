# OpenStarbound Rust Server

A Rust implementation of the OpenStarbound server with full network protocol compatibility.

## Overview

This is a minimal viable implementation of the Starbound server rewritten in Rust. It maintains binary protocol compatibility with the original C++ implementation, allowing existing Starbound clients to connect.

## Protocol Compatibility

- **Protocol Version**: 747 (matches OpenStarbound C++ implementation)
- **Wire Format**: Binary-compatible with the original implementation
- **Packet Types**: Implements core handshake and connection packets

## Features

### Implemented (Phase 1 & 2)
- ✅ TCP server listening on configurable port
- ✅ Protocol version handshake
- ✅ VLQ (Variable Length Quantity) encoding/decoding
- ✅ Basic packet serialization/deserialization
- ✅ Client connection management
- ✅ Protocol version validation
- ✅ **Chat system** (Phase 2)
  - ChatSend/ChatReceive packets
  - Message broadcasting
  - Multiple chat modes (Broadcast, Local, Party)
- ✅ **Admin commands** (Phase 2)
  - `/help` - Show available commands
  - `/players` - List connected players
  - `/nick <name>` - Change nickname
  - `/broadcast <message>` - Server broadcast
  - `/info` - Server information

### To Be Implemented (Future Work)
- ⬜ Packet compression (Zstd) (Phase 3)
- ⬜ World management (Phase 3)
- ⬜ Entity systems (Phase 4)
- ⬜ Player state management (Phase 4)
- ⬜ Universe coordination (Phase 5)
- ⬜ Persistence layer (Phase 5)

## Building

### Prerequisites
- Rust 1.70 or later
- Cargo

### Build Commands

```bash
# Debug build
cargo build

# Release build (optimized)
cargo build --release

# Run tests
cargo test

# Run the server
cargo run
```

## Configuration

The server can be configured via environment variables:

- `SERVER_BIND`: Bind address (default: `0.0.0.0:21025`)
- `MAX_CLIENTS`: Maximum concurrent clients (default: `8`)
- `SERVER_NAME`: Server name shown to clients (default: `OpenStarbound Rust Server`)

Example:
```bash
SERVER_BIND="0.0.0.0:21025" SERVER_NAME="My Rust Server" cargo run
```

## Using the Chat System

Once connected, players can use the following commands:

### Player Commands
- **Send a chat message**: Type normally and press enter
- **Change nickname**: `/nick <new_name>`
- **List players**: `/players`
- **Get server info**: `/info`
- **Get help**: `/help`

### Admin Commands
- **Broadcast message**: `/broadcast <message>` or `/bc <message>`

### Chat Modes
- **Broadcast**: Message sent to all players (default)
- **Local**: Message sent to nearby players only
- **Party**: Message sent to party members only

Example session:
```
> Hello everyone!
[Broadcast] Player1: Hello everyone!

> /nick Alice
Server: Player1 is now known as Alice

> /players
Server: Connected players (2):
Alice (ID: 1)
Bob (ID: 2)

> /info
Server: Server: OpenStarbound Rust Server
Players: 2/8
Protocol Version: 747
```

## Protocol Details

### Packet Wire Format

Each packet follows this format:
1. **Packet Type** (1 byte): Identifies the packet type
2. **Size** (VLQ signed): Packet size in bytes (negative if compressed)
3. **Data**: Packet payload

### VLQ Encoding

Variable Length Quantity encoding is used for efficient integer serialization:
- Small numbers use fewer bytes
- Each byte uses 7 bits for data and 1 bit as a continuation flag
- Signed integers use zigzag encoding: `(n << 1) ^ (n >> 63)`

### Handshake Sequence

1. Client → Server: `ProtocolRequest` (version 747)
2. Server → Client: `ProtocolResponse` (allowed: true/false)
3. If allowed, connection proceeds to authentication

## Testing

The implementation includes unit tests for core protocol functionality:

```bash
# Run all tests
cargo test

# Run tests with output
cargo test -- --nocapture

# Run specific test
cargo test test_vlq_unsigned
```

## Architecture

### Module Structure

- `main.rs`: Server entry point and configuration
- `protocol.rs`: Network protocol implementation (packets, serialization)
- `server.rs`: Server logic (connection handling, packet processing)

### Key Components

- **StarboundServer**: Main server struct managing clients and network
- **Packet Trait**: Interface for packet serialization/deserialization
- **VLQ**: Variable Length Quantity encoder/decoder
- **PacketType**: Enum of all packet types

## Compatibility Notes

This Rust implementation maintains **binary protocol compatibility** with the C++ OpenStarbound implementation. This means:

1. Clients using the C++ implementation can connect to this Rust server
2. The wire protocol is identical at the byte level
3. Protocol version must match (747)

However, note that this is a **minimal viable implementation** focused on demonstrating protocol compatibility. Full game functionality requires implementing all packet types and game logic.

## Development Status

This is a foundational implementation suitable for:
- Demonstrating protocol compatibility
- Basis for incremental feature migration
- Testing and validation of the Rust approach

It is **not yet suitable for production use** as it lacks:
- Complete packet type implementation
- Game logic (worlds, entities, etc.)
- Persistence and state management
- Production-grade error handling and resilience

## Contributing

When extending this implementation:

1. Maintain binary protocol compatibility
2. Add tests for new packet types
3. Follow the existing packet serialization patterns
4. Document protocol-specific behavior

## License

This implementation follows the same license as OpenStarbound.
