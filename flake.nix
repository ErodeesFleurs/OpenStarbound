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

        commonTools = with pkgs; [
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

        clangTools = with pkgs; [
          clang
          lld
        ] ++ commonTools;

        gccTools = with pkgs; [
          gcc
          lld
        ] ++ commonTools;

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

        runtimeLibraryPath = pkgs.lib.makeLibraryPath (
          systemLibraries
          ++ [
            pkgs.stdenv.cc.cc.lib
          ]
        );

        buildClangScript = pkgs.writeShellApplication {
          name = "openstarbound-build-clang";
          runtimeInputs = clangTools ++ systemLibraries;
          text = ''
            export CC=clang
            export CXX=clang++
            export PKG_CONFIG="${pkgs.pkg-config}/bin/pkg-config"
            export VCPKG_ROOT="${vcpkgRoot}"
            export VCPKG_DOWNLOADS="''${VCPKG_DOWNLOADS:-$PWD/.vcpkg/downloads}"
            export VCPKG_DEFAULT_BINARY_CACHE="''${VCPKG_DEFAULT_BINARY_CACHE:-$PWD/.vcpkg/binary-cache}"
            export VCPKG_BINARY_SOURCES="''${VCPKG_BINARY_SOURCES:-clear;files,$VCPKG_DEFAULT_BINARY_CACHE,readwrite}"
            export PKG_CONFIG_PATH="${pkgs.lib.makeSearchPath "lib/pkgconfig" systemLibraries}:''${PKG_CONFIG_PATH:-}"
            export LD_LIBRARY_PATH="${runtimeLibraryPath}:''${LD_LIBRARY_PATH:-}"

            mkdir -p "$VCPKG_DOWNLOADS" "$VCPKG_DEFAULT_BINARY_CACHE"
            cmake --preset=linux-release-clang -S source
            cmake --build build/linux-release-clang
          '';
        };

        buildGccScript = pkgs.writeShellApplication {
          name = "openstarbound-build-gcc";
          runtimeInputs = gccTools ++ systemLibraries;
          text = ''
            export CC=gcc
            export CXX=g++
            export PKG_CONFIG="${pkgs.pkg-config}/bin/pkg-config"
            export VCPKG_ROOT="${vcpkgRoot}"
            export VCPKG_DOWNLOADS="''${VCPKG_DOWNLOADS:-$PWD/.vcpkg/downloads}"
            export VCPKG_DEFAULT_BINARY_CACHE="''${VCPKG_DEFAULT_BINARY_CACHE:-$PWD/.vcpkg/binary-cache}"
            export VCPKG_BINARY_SOURCES="''${VCPKG_BINARY_SOURCES:-clear;files,$VCPKG_DEFAULT_BINARY_CACHE,readwrite}"
            export PKG_CONFIG_PATH="${pkgs.lib.makeSearchPath "lib/pkgconfig" systemLibraries}:''${PKG_CONFIG_PATH:-}"
            export LD_LIBRARY_PATH="${runtimeLibraryPath}:''${LD_LIBRARY_PATH:-}"

            mkdir -p "$VCPKG_DOWNLOADS" "$VCPKG_DEFAULT_BINARY_CACHE"
            cmake --preset=linux-release-gcc -S source
            cmake --build build/linux-release-gcc
          '';
        };

        testClangScript = pkgs.writeShellApplication {
          name = "openstarbound-test-clang";
          runtimeInputs = clangTools ++ systemLibraries;
          text = ''
            export LD_LIBRARY_PATH="${runtimeLibraryPath}:''${LD_LIBRARY_PATH:-}"
            ctest --test-dir build/linux-release-clang --output-on-failure
          '';
        };

        testGccScript = pkgs.writeShellApplication {
          name = "openstarbound-test-gcc";
          runtimeInputs = gccTools ++ systemLibraries;
          text = ''
            export LD_LIBRARY_PATH="${runtimeLibraryPath}:''${LD_LIBRARY_PATH:-}"
            ctest --test-dir build/linux-release-gcc --output-on-failure
          '';
        };
      in
      {
        checks.cmake-presets = pkgs.runCommand "openstarbound-cmake-presets" {
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.python3
          ];
        } ''
          python -m json.tool ${self}/source/CMakePresets.json > /dev/null
          export VCPKG_ROOT="${vcpkgRoot}"
          cmake --list-presets -S ${self}/source > $out
        '';

        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          packages = clangTools ++ systemLibraries;

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
            export LD_LIBRARY_PATH="${runtimeLibraryPath}:''${LD_LIBRARY_PATH:-}"
            mkdir -p "$VCPKG_DOWNLOADS" "$VCPKG_DEFAULT_BINARY_CACHE"

            echo "OpenStarbound clang shell"
            echo "Configure: cmake --preset=linux-release-clang -S source"
            echo "Build:     cmake --build build/linux-release-clang"
          '';
        };

        devShells.gcc = pkgs.mkShell {
          packages = gccTools ++ systemLibraries;

          CC = "gcc";
          CXX = "g++";
          PKG_CONFIG = "${pkgs.pkg-config}/bin/pkg-config";
          VCPKG_ROOT = vcpkgRoot;

          shellHook = ''
            export CC=gcc
            export CXX=g++
            export PKG_CONFIG="${pkgs.pkg-config}/bin/pkg-config"
            export VCPKG_DOWNLOADS="''${VCPKG_DOWNLOADS:-$PWD/.vcpkg/downloads}"
            export VCPKG_DEFAULT_BINARY_CACHE="''${VCPKG_DEFAULT_BINARY_CACHE:-$PWD/.vcpkg/binary-cache}"
            export VCPKG_BINARY_SOURCES="''${VCPKG_BINARY_SOURCES:-clear;files,$VCPKG_DEFAULT_BINARY_CACHE,readwrite}"
            export PKG_CONFIG_PATH="${pkgs.lib.makeSearchPath "lib/pkgconfig" systemLibraries}:''${PKG_CONFIG_PATH:-}"
            export LD_LIBRARY_PATH="${runtimeLibraryPath}:''${LD_LIBRARY_PATH:-}"
            mkdir -p "$VCPKG_DOWNLOADS" "$VCPKG_DEFAULT_BINARY_CACHE"

            echo "OpenStarbound gcc shell"
            echo "Configure: cmake --preset=linux-release-gcc -S source"
            echo "Build:     cmake --build build/linux-release-gcc"
          '';
        };

        apps.build-clang = {
          type = "app";
          program = "${buildClangScript}/bin/openstarbound-build-clang";
          meta.description = "Configure and build OpenStarbound with clang";
        };

        apps.build-gcc = {
          type = "app";
          program = "${buildGccScript}/bin/openstarbound-build-gcc";
          meta.description = "Configure and build OpenStarbound with gcc";
        };

        apps.test-clang = {
          type = "app";
          program = "${testClangScript}/bin/openstarbound-test-clang";
          meta.description = "Run OpenStarbound ctest suite for the clang build";
        };

        apps.test-gcc = {
          type = "app";
          program = "${testGccScript}/bin/openstarbound-test-gcc";
          meta.description = "Run OpenStarbound ctest suite for the gcc build";
        };
      }
    );
}
