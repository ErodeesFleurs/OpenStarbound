{
  description = "C++ devShell";

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
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.llvmPackages.libcxxStdenv; } {
          nativeBuildInputs = with pkgs; [
            pkg-config
            cmake
            ninja
            vcpkg
            clang-tools
          ];

          buildInputs = with pkgs; [
            curl
            zlib
            zstd
            xxHash
            libpng
            freetype
            libopus
            libvorbis
            sdl3
            pkgsLLVM.re2
            cpptrace
            wayland
            libxkbcommon
            egl-wayland
            glew
            libGL
            tzdata
            fleurs-nur.packages.${pkgs.stdenv.hostPlatform.system}.imgui
          ];

          hardeningDisable = [
            "all"
          ];

          env = {
            NIX_CFLAGS_COMPILE = toString [
              "-stdlib=libc++"
              "-Wno-unused-command-line-argument"
              "-B${pkgs.lib.getLib pkgs.libcxx}/lib"
              "-isystem ${pkgs.lib.getDev pkgs.libcxx}/include/c++/v1"
            ];

          };
        };

      }
    );
}
