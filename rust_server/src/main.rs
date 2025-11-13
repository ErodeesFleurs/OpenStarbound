mod protocol;
mod server;

use server::{ServerConfig, StarboundServer};
use log::info;
use std::sync::Arc;
use tokio::signal;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Initialize logger
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    info!("OpenStarbound Rust Server");
    info!("Protocol Version: {}", protocol::PROTOCOL_VERSION);
    
    // Create server configuration
    let config = ServerConfig {
        bind_address: std::env::var("SERVER_BIND")
            .unwrap_or_else(|_| "0.0.0.0:21025".to_string()),
        max_clients: std::env::var("MAX_CLIENTS")
            .unwrap_or_else(|_| "8".to_string())
            .parse()
            .unwrap_or(8),
        server_name: std::env::var("SERVER_NAME")
            .unwrap_or_else(|_| "OpenStarbound Rust Server".to_string()),
    };

    info!("Configuration:");
    info!("  Bind Address: {}", config.bind_address);
    info!("  Max Clients: {}", config.max_clients);
    info!("  Server Name: {}", config.server_name);

    // Create and start server
    let server = Arc::new(StarboundServer::new(config));
    let server_clone = server.clone();

    // Handle Ctrl+C
    tokio::spawn(async move {
        signal::ctrl_c().await.expect("Failed to listen for Ctrl+C");
        info!("Received shutdown signal");
        server_clone.stop().await;
    });

    // Start server
    server.start().await?;

    info!("Server stopped");
    Ok(())
}
