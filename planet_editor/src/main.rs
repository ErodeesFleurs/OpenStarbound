use anyhow::{Context, Result};
use clap::{Parser, Subcommand};
use std::path::PathBuf;

mod world;
mod editor;
mod render;

use world::PlanetMap;

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
    /// Create a new planet map
    New {
        /// Width of the planet map
        #[arg(short, long)]
        width: u32,
        
        /// Height of the planet map
        #[arg(short, long)]
        height: u32,
        
        /// Output file path
        #[arg(short, long)]
        output: PathBuf,
    },
    /// Load and display planet map information
    Info {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,
    },
    /// Render planet map to an image
    Render {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,
        
        /// Output image path
        #[arg(short, long)]
        output: PathBuf,
    },
    /// Edit planet map
    Edit {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,
        
        /// X coordinate
        #[arg(short, long)]
        x: u32,
        
        /// Y coordinate
        #[arg(short, long)]
        y: u32,
        
        /// Material ID to set
        #[arg(short, long)]
        material: u16,
        
        /// Output file path
        #[arg(short, long)]
        output: PathBuf,
    },
}

fn main() -> Result<()> {
    let cli = Cli::parse();

    match &cli.command {
        Commands::New { width, height, output } => {
            println!("Creating new planet map: {}x{}", width, height);
            let planet = PlanetMap::new(*width, *height);
            planet.save(output)
                .context("Failed to save new planet map")?;
            println!("Planet map created successfully at: {}", output.display());
        }
        Commands::Info { input } => {
            println!("Loading planet map from: {}", input.display());
            let planet = PlanetMap::load(input)
                .context("Failed to load planet map")?;
            planet.print_info();
        }
        Commands::Render { input, output } => {
            println!("Rendering planet map from: {}", input.display());
            let planet = PlanetMap::load(input)
                .context("Failed to load planet map")?;
            render::render_to_image(&planet, output)
                .context("Failed to render planet map")?;
            println!("Rendered to: {}", output.display());
        }
        Commands::Edit { input, x, y, material, output } => {
            println!("Editing planet map at ({}, {})", x, y);
            let mut planet = PlanetMap::load(input)
                .context("Failed to load planet map")?;
            editor::set_tile(&mut planet, *x, *y, *material)?;
            planet.save(output)
                .context("Failed to save edited planet map")?;
            println!("Planet map edited and saved to: {}", output.display());
        }
    }

    Ok(())
}
