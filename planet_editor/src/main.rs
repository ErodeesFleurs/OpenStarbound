use anyhow::{Context, Result};
use clap::{Parser, Subcommand};
use std::path::PathBuf;

mod btree;
mod worldfile;
mod render;

use worldfile::WorldFile;

#[derive(Parser)]
#[command(name = "planet_editor")]
#[command(author = "OpenStarbound Contributors")]
#[command(version = "0.1.0")]
#[command(about = "A planet map editor for Starbound worlds", long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Load and display world file information
    Info {
        /// Input .world file path
        #[arg(short, long)]
        input: PathBuf,
    },
    /// Render world map to an image
    Render {
        /// Input .world file path
        #[arg(short, long)]
        input: PathBuf,
        
        /// Output image path
        #[arg(short, long)]
        output: PathBuf,
    },
}

fn main() -> Result<()> {
    let cli = Cli::parse();

    match &cli.command {
        Commands::Info { input } => {
            println!("Loading world file from: {}", input.display());
            let world = WorldFile::load(input)
                .context("Failed to load world file")?;
            
            println!("\nWorld Information:");
            println!("  Dimensions: {}x{} (in tiles)", world.metadata.width, world.metadata.height);
            println!("  Sectors loaded: {}", world.sectors.len());
            
            if let Some(json) = &world.metadata.metadata_json {
                println!("  Metadata: {}", serde_json::to_string_pretty(json)?);
            }
            
            // Count tiles
            let mut total_tiles = 0;
            let mut foreground_count = 0;
            let mut background_count = 0;
            
            for sector in world.sectors.values() {
                for y in 0..worldfile::SECTOR_SIZE {
                    for x in 0..worldfile::SECTOR_SIZE {
                        total_tiles += 1;
                        let tile = sector.tiles[y][x];
                        if tile.foreground != 0 {
                            foreground_count += 1;
                        }
                        if tile.background != 0 {
                            background_count += 1;
                        }
                    }
                }
            }
            
            println!("  Total tiles in loaded sectors: {}", total_tiles);
            println!("  Foreground tiles: {}", foreground_count);
            println!("  Background tiles: {}", background_count);
        }
        Commands::Render { input, output } => {
            println!("Rendering world file from: {}", input.display());
            let world = WorldFile::load(input)
                .context("Failed to load world file")?;
            render::render_to_image(&world, output)
                .context("Failed to render world file")?;
            println!("Rendered to: {}", output.display());
        }
    }

    Ok(())
}
