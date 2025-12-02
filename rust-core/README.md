# Starbound Core (Rust)

Rust implementation of the core types and utilities for OpenStarbound.

This crate provides fundamental data structures that are binary-compatible with the C++ implementation, enabling incremental migration from C++ to Rust.

## Features

- **Math Types**: Vector (`Vec2`, `Vec3`, `Vec4`) and Rectangle (`Rect`, `Box`) types matching C++ `Star::Vector` and `Star::Box`
- **Color**: Full-featured color type with HSV/HSL support, matching C++ `Star::Color`
- **JSON**: JSON value type compatible with C++ `Star::Json` (wraps serde_json)
- **UUID**: Universal unique identifier matching C++ `Star::Uuid`
- **ByteArray**: Binary data container matching C++ `Star::ByteArray`
- **Serialization**: Binary serialization compatible with C++ `StarDataStream` format
- **Error Handling**: Comprehensive error types mirroring C++ exception hierarchy

## Usage

```rust
use starbound_core::{Vec2F, Vec3F, Color, RectF, Json, Uuid, ByteArray};

// Vector operations
let v1 = Vec2F::new(1.0, 2.0);
let v2 = Vec2F::new(3.0, 4.0);
let sum = v1 + v2;
let magnitude = v1.magnitude();

// Color operations
let color = Color::from_hex("#FF8040").unwrap();
let hsv = color.to_hsva();

// Rectangle operations
let rect = RectF::from_coords(0.0, 0.0, 100.0, 100.0);
let center = rect.center();

// JSON operations
let json: Json = r#"{"name": "test", "value": 42}"#.parse().unwrap();
let name = json.get_key("name").unwrap().as_str();

// UUID operations
let uuid = Uuid::new();
println!("{}", uuid.to_string_formatted());

// ByteArray operations
let data = ByteArray::from_slice(&[0xDE, 0xAD, 0xBE, 0xEF]);
println!("{}", data.to_hex()); // "deadbeef"
```

## Binary Compatibility

All types use `#[repr(C)]` to ensure memory layout matches the C++ implementation. This enables:

- FFI interop between Rust and C++ code
- Binary-compatible save file reading/writing
- Network protocol compatibility

## Building

```bash
cd rust-core
cargo build
cargo test
```

## Test Coverage

- **70 unit tests** covering all core functionality
- Math operations (vectors, rectangles)
- Color conversions (RGB, HSV, hex)
- JSON parsing and serialization
- UUID generation and formatting
- ByteArray operations (bitwise, slicing)
- Binary serialization roundtrips

## Migration Strategy

This crate is part of a phased migration plan:

1. **Phase 1** (Complete): Core data types, serialization
   - Vec2, Vec3, Vec4, Rect, Box
   - Color with HSV support
   - JSON value type
   - UUID, ByteArray
   - DataReader/DataWriter

2. **Phase 2** (Next): Network layer, asset loading, Lua bindings
3. **Phase 3**: World generation, entity system
4. **Phase 4**: Renderer, UI, audio

## License

MIT
