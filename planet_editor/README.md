# Planet Map Editor for OpenStarbound

A command-line planet map editor for Starbound worlds, written in Rust.

## Overview

This tool allows you to create, edit, and visualize planet maps for OpenStarbound. It provides a simple JSON-based format for storing planet data and supports rendering maps to PNG images.

## Features

- **Create** new planet maps with custom dimensions
- **View** planet map information and statistics
- **Edit** individual tiles in the map
- **Render** planet maps to PNG images for visualization
- JSON-based storage format for easy inspection and modification
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

The planet editor provides several subcommands for different operations:

### Create a New Planet Map

```bash
planet_editor new --width <WIDTH> --height <HEIGHT> --output <FILE>
```

Example:
```bash
planet_editor new --width 1000 --height 500 --output my_planet.json
```

### View Planet Map Information

```bash
planet_editor info --input <FILE>
```

Example:
```bash
planet_editor info --input my_planet.json
```

Output:
```
Planet Map Information:
  Name: Unnamed Planet
  Dimensions: 1000x500
  Seed: 0
  Total tiles: 500000
  Foreground tiles: 0
  Background tiles: 0
```

### Edit a Tile

```bash
planet_editor edit --input <INPUT_FILE> --x <X> --y <Y> --material <MATERIAL_ID> --output <OUTPUT_FILE>
```

Example:
```bash
planet_editor edit --input my_planet.json --x 100 --y 50 --material 5 --output my_planet_edited.json
```

This sets the foreground material at position (100, 50) to material ID 5.

### Render to Image

```bash
planet_editor render --input <INPUT_FILE> --output <IMAGE_FILE>
```

Example:
```bash
planet_editor render --input my_planet.json --output planet_preview.png
```

This creates a PNG visualization of the planet map where:
- **Red-tinted colors**: Foreground materials (solid ground)
- **Darker colors**: Background materials (caves)
- **Black**: Empty space

## File Format

The planet editor uses a simple JSON format:

```json
{
  "width": 100,
  "height": 100,
  "seed": 0,
  "name": "Unnamed Planet",
  "tiles": [
    {
      "foreground": 0,
      "background": 0,
      "foreground_mod": 0,
      "background_mod": 0
    },
    ...
  ]
}
```

### Fields

- **width**, **height**: Map dimensions in tiles
- **seed**: World generation seed (for reference)
- **name**: Human-readable planet name
- **tiles**: Array of tile data (row-major order)
  - **foreground**: Material ID for the foreground layer (0 = empty)
  - **background**: Material ID for the background layer (0 = empty)
  - **foreground_mod**: Modification/variant for foreground material
  - **background_mod**: Modification/variant for background material

## Material IDs

Material IDs correspond to Starbound's internal material system. Common values:

- `0`: Empty/Air
- `1-100`: Various terrain materials (dirt, stone, ore, etc.)

The exact mapping depends on your Starbound installation and active mods.

## Examples

### Create and Visualize a Simple Planet

```bash
# Create a new 100x100 planet
planet_editor new --width 100 --height 100 --output planet.json

# Add some terrain
planet_editor edit --input planet.json --x 50 --y 50 --material 10 --output planet.json

# Render to see the result
planet_editor render --input planet.json --output planet.png
```

### Batch Processing with Scripts

You can write shell scripts to automate planet editing:

```bash
#!/bin/bash
# Create a planet with a horizontal line of material
planet_editor new --width 100 --height 100 --output planet.json

for x in {0..99}; do
    planet_editor edit --input planet.json --x $x --y 50 --material 5 --output planet.json
done

planet_editor render --input planet.json --output planet_with_line.png
```

## Integration with OpenStarbound

This editor creates a simplified planet format for editing purposes. To use these maps in OpenStarbound:

1. Create and edit your planet map using this tool
2. Export the map data
3. Use OpenStarbound's world generation system to import the data (implementation-dependent)

**Note**: This is a standalone editor and does not directly modify Starbound world files. It's designed for planet design and prototyping.

## Architecture

The editor is organized into several modules:

- **world.rs**: Core data structures for planet maps and tiles
- **editor.rs**: Editing operations (set, fill, copy, etc.)
- **render.rs**: Image rendering functionality
- **main.rs**: CLI interface and command handling

## Performance

The editor is optimized for handling large maps efficiently:

- Uses contiguous memory layout for tile data
- Minimal allocations during editing
- Parallel processing for rendering (where applicable)
- Typical performance: < 1s for 1000x1000 map operations

## Contributing

Contributions are welcome! Areas for improvement:

- Support for liquids and liquid levels
- Biome data integration
- Import/export from Starbound world files
- GUI interface
- More editing operations (copy regions, paste, etc.)

## License

This project is part of OpenStarbound and follows the same license terms.

## See Also

- [OpenStarbound](https://github.com/OpenStarbound/OpenStarbound) - The main project
- [Starbound](https://playstarbound.com/) - The original game
