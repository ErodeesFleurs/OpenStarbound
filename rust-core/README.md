# Starbound Core (Rust)

Rust implementation of the core types and utilities for OpenStarbound.

This crate provides fundamental data structures that are binary-compatible with the C++ implementation, enabling incremental migration from C++ to Rust.

## Features

- **Math Types**: Vector (`Vec2`, `Vec3`, `Vec4`) and Rectangle (`Rect`, `Box`) types matching C++ `Star::Vector` and `Star::Box`
- **Color**: Full-featured color type with HSV/HSL support, matching C++ `Star::Color`
- **Serialization**: Binary serialization compatible with C++ `StarDataStream` format
- **Error Handling**: Comprehensive error types mirroring C++ exception hierarchy

## Usage

```rust
use starbound_core::{Vec2F, Vec3F, Color, RectF};

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

## Migration Strategy

This crate is part of a phased migration plan:

1. **Phase 1** (Current): Core data types, serialization
2. **Phase 2**: Network layer, asset loading, Lua bindings
3. **Phase 3**: World generation, entity system
4. **Phase 4**: Renderer, UI, audio

## License

MIT
