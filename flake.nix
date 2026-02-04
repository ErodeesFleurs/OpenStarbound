{
  description = "Rust devShell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    fleurs-nur.url = "github:ErodeesFleurs/fleurs-nur";
  };

  outputs =
    {
      nixpkgs,
      flake-utils,
      fleurs-nur,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
      {
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          nativeBuildInputs = with pkgs; [
            rust-analyzer
            rustc
            cargo
            pkg-config
            clang
            cmake
            ninja
            vcpkg
          ];

          buildInputs = with pkgs; [
            zlib
            zstd
            libpng
            freetype
            libcpr
            libopus
            libvorbis
            sdl3
            re2
            cpptrace
            wayland
            libxkbcommon
            egl-wayland
            glew
            libGL
            fleurs-nur.packages.${pkgs.stdenv.hostPlatform.system}.imgui
          ];

          env = {
            CC = "clang";
            CXX = "clang++";
          };
        };

      }
    );
}
