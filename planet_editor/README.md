# Planet Map Editor for OpenStarbound

A command-line planet map viewer and analyzer for Starbound worlds, written in Rust.

## Overview

This tool allows you to read and visualize planet map data from OpenStarbound `.world` files. It parses the BTreeDatabase format used by Starbound to store world data and can render maps to PNG images for visualization.

## Features

- **Read** Starbound `.world` files (BTreeDatabase format)
- **View** world information including dimensions, sectors, and tile statistics
- **Render** world maps to PNG images for visualization
- Supports World4 format used by OpenStarbound
- Parses compressed tile sector data
- Fast and memory-efficient implementation in Rust

## Installation

### Building from Source

Make sure you have Rust installed (1.70 or later recommended). Then:

```bash
cd planet_editor
cargo build --release
```

The compiled binary will be at `target/release/planet_editor`.

### Running Tests

```bash
cargo test
```

## Usage

The planet editor provides commands for reading and visualizing Starbound world files:

### View World Information

```bash
planet_editor info --input <WORLD_FILE>
```

Example:
```bash
planet_editor info --input universe/unique-12345.world
```

Output:
```
World Information:
  Dimensions: 2048x1536 (in tiles)
  Sectors loaded: 128
  Total tiles in loaded sectors: 131072
  Foreground tiles: 45231
  Background tiles: 98645
```

### Render World to Image

```bash
planet_editor render --input <WORLD_FILE> --output <IMAGE_FILE>
```

Example:
```bash
planet_editor render --input universe/unique-12345.world --output world_map.png
```

This creates a PNG visualization of the world map where:
- **Red-tinted colors**: Foreground materials (solid ground)
- **Darker colors**: Background materials (caves)
- **Black**: Empty/ungenerated space

**Note**: Only generated sectors (those that have been visited/loaded) will be visible in the rendered image.

## File Format

Starbound world files (`.world`) use the BTreeDatabase format:

- **Format**: Binary BTree database with content identifier "World4"
- **Compression**: Zlib compression for all data values
- **Structure**: 
  - **Metadata**: World dimensions and optional JSON metadata
  - **Tile Sectors**: 32x32 tile arrays stored per sector
  - **Entity Sectors**: Entity data (not currently parsed)
  - **Unique Indices**: Unique entity tracking (not currently parsed)

### Tile Data

Each tile contains:
- **Foreground material** (u16): Material ID for solid blocks
- **Foreground mod** (u16): Color/variant modification
- **Background material** (u16): Material ID for background layer
- **Background mod** (u16): Color/variant modification
- **Liquid** (u8): Liquid type (0 = none)
- **Liquid level** (f32): Amount of liquid
- **Collision**: Collision flags
- **Biome indices**: Block and environment biome information

## Material IDs

Material IDs correspond to Starbound's internal material system:

- `0`: Empty/Air
- `1`: Dirt
- `2`: Stone
- ...and hundreds more depending on game version and mods

The exact mapping is defined in the game's material database.

## Examples

### Analyze a World File

```bash
# View information about a world
planet_editor info --input storage/universe/12345678_-987654321_12345678_1_1.world
```

### Render Multiple Worlds

You can write shell scripts to batch process world files:

```bash
#!/bin/bash
# Render all world files in a directory
for worldfile in storage/universe/*.world; do
    basename=$(basename "$worldfile" .world)
    planet_editor render --input "$worldfile" --output "renders/${basename}.png"
    echo "Rendered $basename"
done
```

## Integration with OpenStarbound

This tool reads actual Starbound `.world` files from your OpenStarbound installation:

1. **Find world files**: Located in `storage/universe/` directory
2. **View or render**: Use this tool to analyze and visualize them
3. **Read-only**: Currently supports reading world files (editing support planned for future)

**File naming**: World files are named using coordinates or unique IDs, e.g.:
- `12345678_-987654321_12345678_1_1.world` (celestial coordinates)
- `unique-instanceworld-abc123.world` (unique/instanced worlds)
- `ClientShipWorld:abc123.shipworld` (player ship worlds)

## Architecture

The editor is organized into several modules:

- **btree.rs**: BTreeDatabase format parser
- **worldfile.rs**: World file data structures and parsing logic
- **render.rs**: Image rendering functionality
- **main.rs**: CLI interface and command handling

## Performance

The editor is optimized for handling large world files efficiently:

- Lazy loading: Only reads needed sectors
- Efficient BTree traversal
- Zlib decompression with buffering
- Typical performance: 
  - Info command: < 1s for most worlds
  - Render command: 1-5s depending on sector count and size

## Contributing

Contributions are welcome! Areas for improvement:

- **World editing**: Write support for modifying world files (currently read-only)
- **Entity parsing**: Read and display entity data from world files
- **Advanced rendering**: Better color schemes, biome visualization
- **GUI interface**: Visual world editor with interactive tools
- **More file types**: Support for `.shipworld` and other variants

## License

This project is part of OpenStarbound and follows the same license terms.

## See Also

- [OpenStarbound](https://github.com/OpenStarbound/OpenStarbound) - The main project
- [Starbound](https://playstarbound.com/) - The original game
