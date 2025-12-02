# Starbound Core (Rust)

Rust implementation of the core types and utilities for OpenStarbound.

This crate provides fundamental data structures that are binary-compatible with the C++ implementation, enabling incremental migration from C++ to Rust.

## Features

- **Math Types**: Vector (`Vec2`, `Vec3`, `Vec4`) and Rectangle (`Rect`, `Box`) types matching C++ `Star::Vector` and `Star::Box`
- **Color**: Full-featured color type with HSV/HSL support, matching C++ `Star::Color`
- **JSON**: JSON value type compatible with C++ `Star::Json` (wraps serde_json)
- **UUID**: Universal unique identifier matching C++ `Star::Uuid`
- **ByteArray**: Binary data container matching C++ `Star::ByteArray`
- **RandomSource**: Deterministic PRNG matching C++ `Star::RandomSource`
- **Perlin Noise**: Coherent noise generation matching C++ `Star::Perlin`
- **AssetPath**: Asset path parsing matching C++ `Star::AssetPath`
- **Encoding**: Hex and Base64 encoding/decoding matching C++ `Star::Encode`
- **SHA-256**: Cryptographic hashing matching C++ `Star::Sha256`
- **Network**: Host address types matching C++ `Star::HostAddress`
- **Compression**: Zlib compress/uncompress matching C++ `Star::Compression`
- **BiMap**: Bidirectional map matching C++ `Star::BiMap`
- **Either**: Left/Right container matching C++ `Star::Either`
- **String Utilities**: String manipulation matching C++ `Star::String`
- **Threading**: Thread utilities, locks, and synchronization primitives matching C++ `Star::Thread`
- **Sockets**: TCP and UDP socket types matching C++ `Star::TcpSocket` and `Star::UdpSocket`
- **Image**: Image manipulation matching C++ `Star::Image`
- **File I/O**: File and buffer operations matching C++ `Star::File` and `Star::IODevice`
- **Lua Bindings**: Lua scripting engine compatible with C++ `Star::Lua`
- **BTree Database**: Persistent key-value storage compatible with C++ `Star::BTreeDatabase`
- **NetElement**: Network synchronization primitives compatible with C++ `Star::NetElement`
- **LRU Cache**: Least-recently-used cache compatible with C++ `Star::LruCache`
- **Worker Pool**: Thread pool for parallel execution compatible with C++ `Star::WorkerPool`
- **Time**: Clock and timer utilities compatible with C++ `Star::Time`, `Star::Clock`, `Star::Timer`
- **Logging**: Multi-sink logging system compatible with C++ `Star::Logger`
- **Option Parser**: Command line argument parsing compatible with C++ `Star::OptionParser`
- **Game Types**: Core game enums and constants compatible with C++ `Star::GameTypes`
- **Damage Types**: Combat damage types compatible with C++ `Star::DamageTypes`
- **Material Types**: Tile material constants compatible with C++ `Star::MaterialTypes`
- **Liquid Types**: Liquid level and store types compatible with C++ `Star::LiquidTypes`
- **Collision Types**: Physics collision types compatible with C++ `Star::CollisionBlock`
- **Item Descriptor**: Item specification compatible with C++ `Star::ItemDescriptor`
- **Celestial Types**: Star system types compatible with C++ `Star::CelestialTypes`
- **Serialization**: Binary serialization compatible with C++ `StarDataStream` format
- **Error Handling**: Comprehensive error types mirroring C++ exception hierarchy

## Usage

```rust
use starbound_core::{
    Vec2F, Vec3F, Color, RectF, Json, Uuid, ByteArray, 
    RandomSource, Perlin, AssetPath, HostAddress, HostAddressWithPort,
    hex_encode, base64_encode, sha256, sha256_hex,
    Thread, TcpSocket, UdpSocket, Image, FileSystem,
    LuaEngine, LuaValue, BTreeDatabase, NetElementVersion,
    LruCache, WorkerPool, Clock, Timer, Logger, LogLevel,
    OptionParser, Direction, Gender, Rarity, DamageType, TeamType
};

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

// Random number generation
let mut rng = RandomSource::with_seed(12345);
let random_float = rng.randf();
let random_int = rng.rand_int_range(1, 100);

// Perlin noise
let noise = Perlin::new(4, 1.0, 1.0, 0.0, 2.0, 2.0, 12345);
let terrain_height = noise.get2(1.5, 2.5);

// Asset paths
let path = AssetPath::split("/assets/image.png:frame1?scale=2");
println!("{}", path.base_path); // "/assets/image.png"

// Encoding
let hex = hex_encode(&[0xDE, 0xAD]);
let b64 = base64_encode(b"Hello");

// Hashing
let hash = sha256_hex(b"data");

// Network addresses
let addr = HostAddressWithPort::parse("127.0.0.1:8080").unwrap();
println!("{}", addr);

// Threading
Thread::sleep(100);
let num_cores = Thread::number_of_processors();

// File operations
let content = FileSystem::read_to_string("config.json")?;
FileSystem::write("output.txt", b"Hello, World!")?;

// LRU Cache
let mut cache = LruCache::new(100);
cache.insert("key", "value");

// Worker Pool
let pool = WorkerPool::new(4);
pool.submit(|| println!("Hello from worker thread!"));
pool.wait();

// Clock and Timer
let clock = Clock::new(true);
std::thread::sleep(std::time::Duration::from_millis(100));
println!("Elapsed: {} ms", clock.milliseconds());

let timer = Timer::with_time(5.0, true);
println!("Time left: {}", timer.time_left(false));

// Logging
Logger::info("Application started");
Logger::debug("Debug message");

// Option Parser
let mut parser = OptionParser::new();
parser.set_command_name("myapp");
parser.add_switch("v", "Verbose mode");
parser.add_parameter("o", "file", RequirementMode::Required, "Output file");

// Game Types
let direction = Direction::Right;
let rarity = Rarity::Legendary;

// Damage Types
let team = EntityDamageTeam::from_type(TeamType::Friendly);
let can_hit = team.can_damage(EntityDamageTeam::from_type(TeamType::Enemy), false);

// Lua Engine
let mut engine = LuaEngine::new(true);
let context = engine.create_context();
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

- **304 unit tests** covering all core functionality
- Math operations (vectors, rectangles)
- Color conversions (RGB, HSV, hex)
- JSON parsing and serialization
- UUID generation and formatting
- ByteArray operations (bitwise, slicing)
- Random number generation (determinism, distributions)
- Perlin noise (continuity, different noise types)
- Asset path parsing
- Encoding (hex, base64)
- SHA-256 hashing (standard test vectors)
- Network addresses (IPv4, IPv6)
- Compression (zlib, multiple levels)
- BiMap (bidirectional mapping)
- Either (left/right container)
- String utilities (case sensitivity, escaping)
- Threading (locks, condition variables, spin locks)
- Sockets (TCP, UDP)
- Image manipulation (pixel formats, blending)
- File I/O (read, write, buffers)
- Lua scripting engine
- BTree database operations
- Network element synchronization
- LRU cache eviction
- Worker pool parallel execution
- Clock and timer operations
- Logging system
- Option parser
- Game type enums
- Damage type calculations
- Binary serialization roundtrips

## Migration Strategy

This crate is part of a phased migration plan:

1. **Phase 1+** (Complete): Core data types, serialization, utilities
   - Vec2, Vec3, Vec4, Rect, Box
   - Color with HSV support
   - JSON value type
   - UUID, ByteArray
   - RandomSource, Perlin noise
   - AssetPath
   - Hex/Base64 encoding
   - SHA-256 hashing
   - HostAddress, HostAddressWithPort
   - Compression (zlib)
   - BiMap, Either
   - String utilities
   - Threading utilities
   - TCP/UDP sockets
   - Image type
   - File I/O
   - DataReader/DataWriter
   - Lua scripting engine
   - BTree database
   - NetElement synchronization
   - LRU cache
   - Worker pool
   - Clock and Timer
   - Logging system
   - Option parser
   - Game types (Direction, Gender, Rarity, etc.)
   - Damage types (DamageType, TeamType, EntityDamageTeam)

2. **Phase 2** (Next): Asset loading system, world storage, Lua script integration
3. **Phase 3**: World generation, entity system, physics
4. **Phase 4**: Renderer (wgpu), UI, audio

## License

MIT
