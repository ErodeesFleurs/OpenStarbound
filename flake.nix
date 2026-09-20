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
            # GCC 16 for the C++20 module in star_core: GCC 15.3 dies on it with
            # an internal compiler error (in tree_node, at cp/module.cc:10037)
            # while 14.2 and 16.2 compile the same file.  Darwin keeps its own
            # stdenv; the module needs a header fallback there until Apple Clang
            # supports C++20 modules.
            stdenv = if pkgs.stdenv.isLinux then pkgs.gcc16Stdenv else pkgs.stdenv;
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
          # ASan + UBSan build: the checkPhase fails on the first sanitizer
          # report, and `nix run .#sanitized-tests` sweeps game_tests with the
          # sanitizers halting on the first error (and a 900s per-case timeout,
          # world generation is much slower under ASan).
          # Note: `nix build .#sanitized` overwrites ./result -- pass
          # `-o result-sanitized` to keep the default package linked there.
          sanitized = openstarbound.override { sanitizers = true; };
          # RelWithAsserts build: no -DNDEBUG, so the project's own invariants
          # (starAssert) and DebugEnabled paths are live while the tests run.
          asserts = openstarbound.override { asserts = true; };
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
        # Same sweep, but against the ASan+UBSan build, with the sanitizers
        # halting on the first report (so a report fails the run), the leak
        # checker enabled, and a 900s per case timeout because world generation
        # is several times slower under ASan.
        sanitized-tests = {
          type = "app";
          program = "${pkgs.writeShellScript "openstarbound-sanitized-tests" ''
            export ASAN_OPTIONS="detect_leaks=1''${ASAN_OPTIONS:+:$ASAN_OPTIONS}"
            export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1''${UBSAN_OPTIONS:+:$UBSAN_OPTIONS}"
            export OPENSTARBOUND_TEST_TIMEOUT="''${OPENSTARBOUND_TEST_TIMEOUT:-900}"
            exec ${(packagesFor pkgs).sanitized}/bin/run-game-tests "$@"
          ''}";
          meta.description = "Run OpenStarbound's game_tests with AddressSanitizer, UndefinedBehaviorSanitizer and the leak checker";
        };
        # Same sweep against the assert-enabled build, so a violated invariant
        # fails the run instead of being compiled out.
        asserts-tests = {
          type = "app";
          program = "${pkgs.writeShellScript "openstarbound-asserts-tests" ''
            export OPENSTARBOUND_TEST_TIMEOUT="''${OPENSTARBOUND_TEST_TIMEOUT:-900}"
            exec ${(packagesFor pkgs).asserts}/bin/run-game-tests "$@"
          ''}";
          meta.description = "Run OpenStarbound's game_tests with the project's own assertions enabled";
        };
      });

      # `nix develop` is for iterating on the C++ tree with cmake/ninja against
      # the repository's own CMakeLists.txt.  LD_LIBRARY_PATH is needed because
      # upstream disables CMake's build RPATH, so binaries built by hand cannot
      # locate their dependencies otherwise: the dependencies (and their runtime
      # closures, which lld linked binaries reference directly) are resolved by
      # the shell hook from the library outputs.  The shell deliberately does not
      # depend on the package, so that editing the sources does not rebuild it.
      #
      # Hand builds should additionally pass
      #   -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld
      # because linking the test binaries (750MB / 1.8GB sanitized) takes
      # minutes with the default linker and seconds with lld.  For sanitizer
      # trees use -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-g1 -fsanitize=..."
      # (Release has no -g, and the per configuration flags are set with set() in
      # the CMake files, so they cannot be overridden from the command line).
      devShells = forAllSystems (
        pkgs:
        let
          # Library outputs to take the runtime library path from.  The package's
          # buildInputs carry the dev outputs of most of these (headers only), so
          # they are named here and mapped through getLib.
          shellLibraryPackages = with pkgs; [
            zlib-ng
            zlib
            libcpr
            libpng
            freetype
            libogg
            libvorbis
            zstd
            libopus
            re2
            cpptrace
            sdl3
            glew
            libGL
            wayland
            libxkbcommon
          ];
          shellFor =
            package:
            pkgs.mkShell {
              inputsFrom = [ package ];
              shellHook = ''
                shell_library_path=""
                for lib in ${nixpkgs.lib.concatStringsSep " " (map (d: "${pkgs.lib.getLib d}") shellLibraryPackages)}; do
                  for path in $(nix-store -qR "$lib"); do
                    [ -d "$path/lib" ] && shell_library_path="$shell_library_path:$path/lib"
                  done
                done
                export LD_LIBRARY_PATH="''${shell_library_path#:}''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
              '';
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
