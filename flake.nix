{
  description = "OpenStarbound — build the client, server and asset tools with Nix";

  # Pinned to the revision the C++ reference build was validated against.
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/c8d3b8cdb3959a35bb0af0de111b7d13caf79896";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];

      forAllSystems =
        f:
        nixpkgs.lib.genAttrs systems (
          system:
          f (import nixpkgs {
            inherit system;
            # The Steam and Discord game SDKs vendored in lib/linux are unfree;
            # without this, `packages.full`, `devShells.full` and therefore
            # `nix flake check` cannot be evaluated at all.  Nothing else is
            # allowed to be unfree.
            config.allowUnfreePredicate = pkg: (pkg.pname or pkg.name or "") == "openstarbound-sdks";
          })
        );

      # Ends up in the version string the binaries report and log.
      sourceIdentifier = self.shortRev or self.dirtyShortRev or "dirty";

      packagesFor =
        pkgs:
        let
          openstarbound = pkgs.callPackage ./nix/openstarbound.nix {
            # The build inputs are source/, assets/, scripts/, cmake/ and lib/.
            # Keep the flake's own files out of the source tree so that editing
            # them does not force a rebuild of the ten minute C++ build.
            src = nixpkgs.lib.cleanSourceWith {
              src = ./.;
              name = "openstarbound-source";
              filter =
                path: _type:
                !builtins.elem (baseNameOf path) [
                  "flake.nix"
                  "flake.lock"
                  "nix"
                ];
            };
            inherit sourceIdentifier;
          };
        in
        {
          inherit openstarbound;
          default = openstarbound;
          # Headless server: no SDL3/OpenGL/GLEW, no client.
          server = openstarbound.override { guiSupport = false; };
          # Everything the vendored SDKs can enable.  The Steam and Discord
          # SDKs are unfree, so this needs NIXPKGS_ALLOW_UNFREE=1 --impure.
          full = openstarbound.override {
            steamSupport = true;
            discordSupport = true;
            qtSupport = true;
          };
        };
    in
    {
      packages = forAllSystems packagesFor;

      # `nix run .#game-tests -- --gtest_filter=...` runs the asset-dependent
      # game_tests against a local copy of Starbound's assets and writes a
      # report; those assets are gitignored, so they cannot be a build input.
      apps = forAllSystems (pkgs: {
        game-tests = {
          type = "app";
          program = "${(packagesFor pkgs).default}/bin/run-game-tests";
          meta.description = "Run OpenStarbound's game_tests against your Starbound assets";
        };
      });

      # `nix develop` is for iterating on the C++ tree with cmake/ninja against
      # the repository's own (unmodified) CMakeLists.txt.  LD_LIBRARY_PATH is
      # needed because upstream disables CMake's build RPATH, so binaries built
      # by hand cannot locate their dependencies otherwise.
      devShells = forAllSystems (
        pkgs:
        let
          shellFor =
            package:
            pkgs.mkShell {
              inputsFrom = [ package ];
              env.LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath package.buildInputs;
              # Upstream's ImGui Lua bindings hand Lua-supplied strings to ImGui
              # as printf format strings, which NixOS' `format` hardening turns
              # into -Werror=format-security.  The packaged build applies
              # nix/patches/imgui-lua-literal-text.patch instead; this shell
              # builds the tree as-is, like upstream CI (which does not enable
              # that warning in the first place).
              hardeningDisable = [ "format" ];
            };
        in
        {
          default = shellFor (packagesFor pkgs).default;
          full = shellFor (packagesFor pkgs).full;
        }
      );

      checks = forAllSystems (pkgs: {
        default = (packagesFor pkgs).default;
      });

      formatter = forAllSystems (pkgs: pkgs.nixfmt);
    };
}
