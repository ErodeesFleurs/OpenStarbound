# OpenStarbound Rust Server

A Rust implementation of the OpenStarbound server with full network protocol compatibility.

## Overview

This is a minimal viable implementation of the Starbound server rewritten in Rust. It maintains binary protocol compatibility with the original C++ implementation, allowing existing Starbound clients to connect.

## Protocol Compatibility

- **Protocol Version**: 747 (matches OpenStarbound C++ implementation)
- **Wire Format**: Binary-compatible with the original implementation
- **Packet Types**: Implements core handshake and connection packets

## Features

### Implemented
- ✅ TCP server listening on configurable port
- ✅ Protocol version handshake
- ✅ VLQ (Variable Length Quantity) encoding/decoding
- ✅ Basic packet serialization/deserialization
- ✅ Client connection management
- ✅ Protocol version validation

### To Be Implemented (Future Work)
- ⬜ Full packet type support (currently only handshake packets)
- ⬜ Packet compression (Zstd)
- ⬜ World management
- ⬜ Entity systems
- ⬜ Player state management
- ⬜ Chat system
- ⬜ Universe coordination
- ⬜ Persistence layer

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
