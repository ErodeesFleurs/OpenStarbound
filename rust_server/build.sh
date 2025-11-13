#!/bin/bash
# Build script for OpenStarbound Rust Server

set -e  # Exit on error

echo "Building OpenStarbound Rust Server..."
echo "======================================"

# Check if cargo is available
if ! command -v cargo &> /dev/null; then
    echo "Error: cargo not found. Please install Rust from https://rustup.rs/"
    exit 1
fi

# Display Rust version
echo "Rust version: $(rustc --version)"
echo "Cargo version: $(cargo --version)"
echo ""

# Parse command line arguments
BUILD_TYPE="${1:-debug}"

if [ "$BUILD_TYPE" = "release" ]; then
    echo "Building in RELEASE mode (optimized)..."
    cargo build --release
    echo ""
    echo "Build complete!"
    echo "Binary: target/release/starbound_server_rust"
elif [ "$BUILD_TYPE" = "debug" ]; then
    echo "Building in DEBUG mode..."
    cargo build
    echo ""
    echo "Build complete!"
    echo "Binary: target/debug/starbound_server_rust"
elif [ "$BUILD_TYPE" = "test" ]; then
    echo "Running tests..."
    cargo test
elif [ "$BUILD_TYPE" = "clean" ]; then
    echo "Cleaning build artifacts..."
    cargo clean
    echo "Clean complete!"
else
    echo "Usage: $0 [debug|release|test|clean]"
    echo "  debug   - Build debug version (default)"
    echo "  release - Build optimized release version"
    echo "  test    - Run tests"
    echo "  clean   - Clean build artifacts"
    exit 1
fi
