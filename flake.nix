{
  description = "OpenStarbound clang development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachSystem [ "x86_64-linux" ] (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        vcpkgRoot = "${pkgs.vcpkg}/share/vcpkg";

        nativeTools = with pkgs; [
          clang
          lld
          sccache
          cmake
          ninja
          pkg-config
          vcpkg
          git
          python3
          python3Packages.jinja2
          perl
          curl
          cacert
          zip
          unzip
          gnutar
          gnumake
          autoconf
          automake
          autoconf-archive
          libtool
          bison
          flex
          meson
          nasm
        ];

        systemLibraries = with pkgs; [
          alsa-lib
          dbus
          fontconfig
          freetype
          glew
          ibus
          libGL
          libGLU
          libpng
          libxkbcommon
          wayland
          wayland-protocols
          libx11
          libxcursor
          libxext
          libxi
          libxinerama
          libxmu
          libxrandr
          libxrender
          libxscrnsaver
          libxtst
          libxxf86vm
        ];

        buildScript = pkgs.writeShellApplication {
          name = "openstarbound-build-clang";
          runtimeInputs = nativeTools ++ systemLibraries;
          text = ''
            export CC=clang
            export CXX=clang++
            export PKG_CONFIG="${pkgs.pkg-config}/bin/pkg-config"
            export VCPKG_ROOT="${vcpkgRoot}"
            export VCPKG_DOWNLOADS="''${VCPKG_DOWNLOADS:-$PWD/.vcpkg/downloads}"
            export VCPKG_DEFAULT_BINARY_CACHE="''${VCPKG_DEFAULT_BINARY_CACHE:-$PWD/.vcpkg/binary-cache}"
            export VCPKG_BINARY_SOURCES="''${VCPKG_BINARY_SOURCES:-clear;files,$VCPKG_DEFAULT_BINARY_CACHE,readwrite}"
            export PKG_CONFIG_PATH="${pkgs.lib.makeSearchPath "lib/pkgconfig" systemLibraries}:''${PKG_CONFIG_PATH:-}"

            mkdir -p "$VCPKG_DOWNLOADS" "$VCPKG_DEFAULT_BINARY_CACHE"
            cmake --preset=linux-release-clang -S source
            cmake --build build/linux-release-clang
          '';
        };
      in
      {
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          packages = nativeTools ++ systemLibraries;

          CC = "clang";
          CXX = "clang++";
          PKG_CONFIG = "${pkgs.pkg-config}/bin/pkg-config";
          VCPKG_ROOT = vcpkgRoot;

          shellHook = ''
            export CC=clang
            export CXX=clang++
            export PKG_CONFIG="${pkgs.pkg-config}/bin/pkg-config"
            export VCPKG_DOWNLOADS="''${VCPKG_DOWNLOADS:-$PWD/.vcpkg/downloads}"
            export VCPKG_DEFAULT_BINARY_CACHE="''${VCPKG_DEFAULT_BINARY_CACHE:-$PWD/.vcpkg/binary-cache}"
            export VCPKG_BINARY_SOURCES="''${VCPKG_BINARY_SOURCES:-clear;files,$VCPKG_DEFAULT_BINARY_CACHE,readwrite}"
            export PKG_CONFIG_PATH="${pkgs.lib.makeSearchPath "lib/pkgconfig" systemLibraries}:''${PKG_CONFIG_PATH:-}"
            mkdir -p "$VCPKG_DOWNLOADS" "$VCPKG_DEFAULT_BINARY_CACHE"

            echo "OpenStarbound clang shell"
            echo "Configure: cmake --preset=linux-release-clang -S source"
            echo "Build:     cmake --build build/linux-release-clang"
          '';
        };

        apps.build-clang = {
          type = "app";
          program = "${buildScript}/bin/openstarbound-build-clang";
          meta.description = "Configure and build OpenStarbound with clang";
        };
      }
    );
}
