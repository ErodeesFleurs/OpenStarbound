{
  lib,
  stdenv,
  stdenvNoCC,
  callPackage,
  cmake,
  ninja,
  pkg-config,
  autoPatchelfHook,
  zlib-ng,
  libcpr,
  libpng,
  freetype,
  libogg,
  libvorbis,
  zstd,
  libopus,
  re2,
  cpptrace,
  lld,
  sdl3,
  glew,
  libGL,
  wayland,
  libxkbcommon,
  qt5,
  # This repository's source tree; the CMake layout expects the repository root
  # (source/CMakeLists.txt is configured through `cmakeDir`).
  src,
  # Recorded into the built-in version string (shown in logs and the window
  # title); normally the git revision the tree was built from.
  sourceIdentifier ? "unknown",
  guiSupport ? true,
  steamSupport ? false,
  discordSupport ? false,
  qtSupport ? false,
  withTests ? true,
  # Build with AddressSanitizer + UndefinedBehaviorSanitizer.  The checkPhase
  # then fails on the first report, so `nix build .#sanitized` is a hard gate
  # for undefined behaviour in the code the tests cover; the game_tests sweep
  # runs through `nix run .#sanitized-tests`.
  sanitizers ? false,
  # Build with the project's own assertions live.  RelWithAsserts leaves NDEBUG
  # undefined, so starAssert and the DebugEnabled code paths actually execute in
  # the checkPhase and in `nix run .#asserts-tests`; the shipped configurations
  # all define NDEBUG, so nothing else ever runs them.
  asserts ? false,
}:

let
  # Version lived in source/core/StarVersion.cpp.in.  It must match the
  # "version" field in assets/opensb/_metadata or Root refuses to load assets.
  version = "0.1.15.1";
  sdkExtension = stdenv.hostPlatform.extensions.sharedLibrary;
  imgui = callPackage ./imgui.nix { };
  opusCmake = callPackage ./opus-cmake-config.nix { };

  # Optional prebuilt Steam/Discord SDKs, vendored under lib/linux in the
  # repository.  They are unfree redistribution-wise and x86_64 only, hence the
  # asserts below; they need patchelf'ing because they are prebuilt binaries.
  sdks = stdenvNoCC.mkDerivation {
    pname = "openstarbound-sdks";
    inherit version;
    dontUnpack = true;
    dontConfigure = true;
    dontBuild = true;
    strictDeps = true;
    nativeBuildInputs = [ autoPatchelfHook ];
    buildInputs = [ stdenv.cc.cc.lib ];
    installPhase = ''
      runHook preInstall
      mkdir -p "$out/lib"
    ''
    + lib.optionalString steamSupport ''
      install -m755 ${src}/lib/linux/libsteam_api${sdkExtension} "$out/lib/"
      mkdir -p "$out/include"
      cp -r ${src}/lib/linux/include/steam "$out/include/"
    ''
    + lib.optionalString discordSupport ''
      install -m755 ${src}/lib/linux/libdiscord_game_sdk${sdkExtension} "$out/lib/"
    ''
    + ''
      runHook postInstall
    '';
    meta = {
      description = "Steam and Discord game SDKs vendored in the OpenStarbound repository";
      license = lib.licenses.unfree;
      platforms = [ "x86_64-linux" ];
    };
  };

  # Everything upstream drops into dist/.  starbound/starbound_server need
  # assets, so they are launched through nix/launcher.sh instead of being
  # exposed directly.
  wrapped = [ "starbound_server" ] ++ lib.optional guiSupport "starbound";
  plain =
    [
      "asset_packer"
      "asset_unpacker"
      "btree_repacker"
      "dump_versioned_json"
      "make_versioned_json"
    ]
    ++ lib.optionals withTests [ "core_tests" "game_tests" ];
  qtTools = lib.optional qtSupport "json_tool" ++ lib.optional (qtSupport && steamSupport) "mod_uploader";
  distBinaries = plain ++ wrapped ++ qtTools;
in
assert lib.assertMsg (stdenv.hostPlatform.isLinux && stdenv.hostPlatform.is64bit)
  "This flake only supports x86_64/aarch64 Linux.";
assert lib.assertMsg (stdenv.buildPlatform == stdenv.hostPlatform)
  "OpenStarbound's build system does not support cross compilation here.";
assert lib.assertMsg (!(steamSupport || discordSupport) || stdenv.hostPlatform.isx86_64)
  "The vendored Linux Steam/Discord SDK binaries are x86_64 only.";

stdenv.mkDerivation {
  pname = if sanitizers then "openstarbound-sanitized" else "openstarbound";
  inherit version src;

  # Deviation from upstream, taken from the verified reference build: the Lua
  # binding passed Lua-supplied strings to ImGui as printf format strings.
  patches = [ ./patches/imgui-lua-literal-text.patch ];

  strictDeps = true;
  # lld is only there for hand builds (see the devShells in flake.nix):
  # `-fuse-ld=lld` links the ~750MB test binaries in seconds instead of minutes.
  # The package itself keeps the default linker: linked with lld, the installed
  # binaries no longer found their dependencies through CMake's build RPATH
  # (libz-ng.so.2 went missing in the test phase), which GNU ld provides.
  nativeBuildInputs = [ cmake ninja pkg-config lld ] ++ lib.optional qtSupport qt5.wrapQtAppsHook;
  buildInputs = [
    zlib-ng
    libcpr
    libpng
    freetype
    libogg
    libvorbis
    zstd
    imgui
    libopus
    opusCmake
    re2
    cpptrace
  ]
  ++ lib.optionals guiSupport [
    sdl3
    glew
    libGL
    wayland
    libxkbcommon
  ]
  ++ lib.optional (steamSupport || discordSupport) sdks
  ++ lib.optional qtSupport qt5.qtbase;

  # Dependency discovery adaptations for nixpkgs, no upstream source change:
  # CMake ships no `GLEW::glew_s` target, and upstream disables the build RPATH
  # even though these binaries are copied out of the build tree instead of being
  # installed by CMake.
  postPatch = ''
    substituteInPlace source/CMakeLists.txt \
      --replace-fail '$<IF:$<TARGET_EXISTS:GLEW::glew_s>,GLEW::glew_s,GLEW>' \
        ${lib.escapeShellArg "\${GLEW_LIBRARY}"} \
      --replace-fail 'set(CMAKE_SKIP_BUILD_RPATH TRUE)' \
        'set(CMAKE_SKIP_BUILD_RPATH FALSE)'
  ''
  # Linking the ~750MB test binaries with the default GNU ld takes minutes per
  # link (the sanitized ones are ~1.8GB and took 7), lld does it in seconds.
  + lib.optionalString (sanitizers || asserts) ''
    # Line tables only: it keeps the file:line in sanitizer and valgrind reports
    # but cuts the binaries and their link time down a lot.  The per-configuration
    # flags are set() unconditionally in the CMake files, so they have to be
    # patched rather than overridden.  RelWithAsserts deliberately keeps NDEBUG
    # off, so its flags carry no -DNDEBUG to patch.
    substituteInPlace source/CMakeLists.txt \
      --replace-fail 'set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -DNDEBUG -O3 -ffast-math")' \
        'set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g1 -DNDEBUG -O3 -ffast-math")' \
      --replace-fail 'set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -DNDEBUG -O3 -ffast-math")' \
        'set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g1 -DNDEBUG -O3 -ffast-math")' \
      --replace-fail 'set(CMAKE_C_FLAGS_RELWITHASSERTS "-g -O3 -ffast-math")' \
        'set(CMAKE_C_FLAGS_RELWITHASSERTS "-g1 -O3 -ffast-math")' \
      --replace-fail 'set(CMAKE_CXX_FLAGS_RELWITHASSERTS "-g -O3 -ffast-math")' \
        'set(CMAKE_CXX_FLAGS_RELWITHASSERTS "-g1 -O3 -ffast-math")'
  '';

  cmakeDir = "../source";
  # RelWithAsserts is the only configuration without -DNDEBUG, i.e. the only one
  # where starAssert and DebugEnabled are compiled in.
  cmakeBuildType = if asserts then "RelWithAsserts" else "RelWithDebInfo";
  cmakeFlags = [
    (lib.cmakeFeature "STAR_SOURCE_IDENTIFIER" sourceIdentifier)
    (lib.cmakeBool "STAR_BUILD_GUI" guiSupport)
    # The maintenance utilities (map_grep and the tileset tools) are commented
    # out in the CMake files; building them here keeps them from rotting.
    (lib.cmakeBool "STAR_BUILD_DEV_TOOLS" true)
    (lib.cmakeBool "BUILD_TESTING" withTests)
    (lib.cmakeBool "STAR_ENABLE_STEAM_INTEGRATION" steamSupport)
    (lib.cmakeBool "STAR_ENABLE_DISCORD_INTEGRATION" discordSupport)
    (lib.cmakeBool "STAR_BUILD_QT_TOOLS" qtSupport)
  ]
  ++ lib.optionals steamSupport [
    (lib.cmakeFeature "STEAM_API_INCLUDE_DIR" "${sdks}/include")
    (lib.cmakeFeature "STEAM_API_LIBRARY" "${sdks}/lib/libsteam_api${sdkExtension}")
  ]
  ++ lib.optionals discordSupport [
    (lib.cmakeFeature "DISCORD_API_LIBRARY" "${sdks}/lib/libdiscord_game_sdk${sdkExtension}")
  ]
  ++ lib.optionals sanitizers [
    # Note: cmakeFlags elements end up in a shell command, so they must not
    # contain spaces.  The sanitizer runtimes have to be linked by the compiler
    # driver, going through NIX_LDFLAGS would hand -fsanitize=... to ld itself.
    # 'enum' is not part of -fsanitize=undefined and catches loads of enum
    # values that no enumerator maps to (uninitialised enum members, for
    # example); signed-integer-overflow is not in it either.
    "-DSTAR_USE_JEMALLOC=false"
    "-DCMAKE_C_FLAGS=-fsanitize=address,undefined,enum,signed-integer-overflow"
    "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined,enum,signed-integer-overflow"
    "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined,enum,signed-integer-overflow"
  ];

  # core_tests carries the "NoAssets" label and runs without any game assets.
  # game_tests needs a user's proprietary assets and writable storage, so it is
  # built and installed but never run here: the package ships
  # bin/run-game-tests (nix run .#game-tests) for that.
  #
  # The sanitized build also runs the leak checker; core_tests holds on to
  # nothing that LeakSanitizer reports (it used to keep ~22KB alive through a
  # Lua reference cycle in the require test).
  doCheck = withTests;
  checkPhase = ''
    runHook preCheck
    ${lib.optionalString sanitizers "ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 "}ctest --output-on-failure -L NoAssets
    runHook postCheck
  '';

  dontWrapQtApps = true;
  installPhase = ''
    runHook preInstall
    mkdir -p "$out/bin" "$out/libexec" "$out/share/openstarbound/assets"

    for binary in ${lib.escapeShellArgs distBinaries}; do
      install -m755 "../dist/$binary" "$out/libexec/$binary"
    done
    for binary in ${lib.escapeShellArgs plain}; do
      ln -s "../libexec/$binary" "$out/bin/$binary"
    done
    ${lib.concatMapStrings (binary: ''
      install -m755 ${./launcher.sh} "$out/bin/${binary}"
      substituteInPlace "$out/bin/${binary}" \
        --replace-fail '@binary@' "$out/libexec/${binary}" \
        --replace-fail '@bundledAssets@' "$out/share/openstarbound/assets"
    '') wrapped}
    ${lib.optionalString qtSupport ''
      for binary in ${lib.escapeShellArgs qtTools}; do
        install -m755 "../dist/$binary" "$out/bin/$binary"
      done
    ''}
    ${lib.optionalString withTests ''
      # Runs game_tests against assets from the local filesystem; they cannot be
      # a build input because they are gitignored and not redistributable.
      install -m755 ${./run-game-tests.sh} "$out/bin/run-game-tests"
      substituteInPlace "$out/bin/run-game-tests" \
        --replace-fail '@gameTests@' "$out/libexec/game_tests" \
        --replace-fail '@bundledAssets@' "$out/share/openstarbound/assets"
    ''}
    cp -r ../assets/opensb "$out/share/openstarbound/assets/opensb"
    install -m644 ../scripts/packing.config "$out/share/openstarbound/packing.config"
    install -m644 ../scripts/ci/linux/sbinit.config \
      "$out/share/openstarbound/sbinit.config.example"
    runHook postInstall
  '';

  postFixup = lib.concatMapStrings (binary: ''
    wrapQtApp "$out/bin/${binary}"
  '') qtTools;

  passthru = {
    inherit imgui;
    inherit
      guiSupport
      steamSupport
      discordSupport
      qtSupport
      withTests
      distBinaries
      ;
  };

  meta = {
    description = "Starbound client and server, the OpenStarbound fork";
    homepage = "https://github.com/OpenStarbound/OpenStarbound";
    platforms = [ "x86_64-linux" "aarch64-linux" ];
    mainProgram = if guiSupport then "starbound" else "starbound_server";
    # Upstream carries no top-level license grant; do not invent one.
  };
}
