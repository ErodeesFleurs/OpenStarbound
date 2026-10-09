{
  pkgs,
  sanitizers ? false,
}:

let
  inherit (pkgs) lib;
  cc = pkgs.callPackage ./compiler.nix { inherit sanitizers; };
  stdenv = pkgs.overrideCC pkgs.stdenv cc;

  # ASan's instrumented libc++ is selected by the compiler wrapper. All C++
  # code in the executable must be instrumented with the same container ABI.
  instrument =
    package:
    package.overrideAttrs (old: {
      env = (old.env or { }) // {
        XCLANG_ROOT = cc.bundledRuntime;
      };
      cmakeFlags = (old.cmakeFlags or [ ]) ++ [
        (lib.cmakeFeature "CMAKE_C_COMPILER" "${cc}/bin/clang")
        (lib.cmakeFeature "CMAKE_CXX_COMPILER" "${cc}/bin/clang++")
        (lib.cmakeBool "CMAKE_SKIP_BUILD_RPATH" false)
        (lib.cmakeFeature "CMAKE_BUILD_RPATH" (
          lib.makeLibraryPath ((old.buildInputs or [ ]) ++ (old.propagatedBuildInputs or [ ]))
        ))
      ];
      preConfigure =
        lib.optionalString sanitizers ''
          export CFLAGS="''${CFLAGS-} -fsanitize=address,undefined"
          export CXXFLAGS="''${CXXFLAGS-} -fsanitize=address,undefined"
          export LDFLAGS="''${LDFLAGS-} -fsanitize=address,undefined"
        ''
        + (old.preConfigure or "");
    });

  # Test helpers have C++ interfaces too; using nixpkgs' shared gtest here
  # would introduce a second, incompatible C++ runtime into dependency tests.
  gtest = instrument (
    pkgs.gtest.override {
      inherit stdenv;
      static = true;
    }
  );
  gbenchmark = instrument (
    (pkgs.gbenchmark.override { inherit stdenv gtest; }).overrideAttrs (old: {
      cmakeFlags = (old.cmakeFlags or [ ]) ++ [
        (lib.cmakeBool "BUILD_SHARED_LIBS" false)
        # Remove compiler-instruction snapshots; retain all behavioral tests.
        (lib.cmakeBool "BENCHMARK_ENABLE_ASSEMBLY_TESTS" false)
      ];
    })
  );
  abseil-cpp = instrument (
    pkgs.abseil-cpp_202601.override {
      inherit stdenv gtest;
      static = true;
      cxxStandard = "20";
    }
  );

  # ICU exposes C++ APIs used by RE2. Keep both its libraries and its data
  # archive static, without rebuilding unrelated C-ABI libraries in nixpkgs.
  # Do not use withStatic: that splits archives into a separate output while
  # retaining shared libraries, which find_package(ICU) would prefer.
  icu = instrument (
    (pkgs.icu.override { inherit stdenv; }).overrideAttrs (old: {
      dontDisableStatic = true;
      configureFlags = (old.configureFlags or [ ]) ++ [
        "--enable-static"
        "--disable-shared"
      ];
    })
  );
  re2 = instrument (
    (pkgs.re2.override {
      inherit
        stdenv
        abseil-cpp
        icu
        gtest
        gbenchmark
        ;
    }).overrideAttrs
      (old: {
        cmakeFlags = (old.cmakeFlags or [ ]) ++ [ (lib.cmakeBool "BUILD_SHARED_LIBS" false) ];
        # FindICU's uc target does not carry the static data archive. Export the
        # data component as well, so downstream CMake consumers link it too.
        postPatch = (old.postPatch or "") + ''
          substituteInPlace CMakeLists.txt re2Config.cmake.in \
            --replace-fail "ICU REQUIRED COMPONENTS uc)" "ICU REQUIRED COMPONENTS uc data)"
          substituteInPlace CMakeLists.txt \
            --replace-fail "if(NOT TARGET ICU::uc)" "if(NOT TARGET ICU::uc OR NOT TARGET ICU::data)" \
            --replace-fail "target_link_libraries(re2 PUBLIC ICU::uc)" \
              "target_link_libraries(re2 PUBLIC ICU::uc ICU::data)"
        '';
      })
  );
  libcpr = instrument (
    pkgs.libcpr.override {
      inherit stdenv;
      staticOnly = true;
    }
  );
  cpptrace = instrument (
    (pkgs.cpptrace.override {
      inherit stdenv gtest;
      static = true;
    }).overrideAttrs
      (old: {
        # Use the compiler's private unwinder, not a second shared libunwind.
        buildInputs = [ (lib.getDev pkgs.libdwarf) ];
        propagatedBuildInputs = [
          pkgs.zstd
          pkgs.libdwarf
        ];
        cmakeFlags = (old.cmakeFlags or [ ]) ++ [
          (lib.cmakeBool "CPPTRACE_UNWIND_WITH_LIBUNWIND" false)
          (lib.cmakeBool "CPPTRACE_UNWIND_WITH_UNWIND" true)
        ];
      })
  );

  # Qt's public APIs and plugin interfaces are C++, so Qt libraries AND its
  # platform/image/style plugins must be linked statically into each tool.
  # Qtbase contains moc/uic/rcc. Only Linguist's lupdate/lrelease family from
  # qttools is needed; QDoc would add nixpkgs' incompatible shared LLVM C++ API.
  qt5 = (pkgs.qt5.override { inherit stdenv; }).overrideScope (
    final: prev: {
      qtbase = instrument (
        (prev.qtbase.override {
          inherit icu;
          # Avoid the translations -> qttools -> qtbase bootstrap cycle; the
          # OpenStarbound widgets do not install or load Qt translation catalogs.
          qttranslations = null;
        }).overrideAttrs
          (old: {
            propagatedBuildInputs = (old.propagatedBuildInputs or [ ]) ++ [ pkgs.zstd ];
            patches = (old.patches or [ ]) ++ [ ../cmake/patches/qtbase-libcxx.patch ];
            # Qt's shared-library RPATH feature is invalid for a static Qt build.
            configureFlags =
              lib.filter (flag: flag != "-shared" && flag != "-rpath") (old.configureFlags or [ ])
              ++ [
                "-static"
                "-no-rpath"
                "-platform"
                "linux-clang"
              ];
            # Host tools still load C-ABI dependencies through the bundled lld.
            preConfigure = (old.preConfigure or "") + ''
              export NIX_LDFLAGS="''${NIX_LDFLAGS-} -rpath ${
                lib.makeLibraryPath (
                  (old.buildInputs or [ ]) ++ (old.propagatedBuildInputs or [ ]) ++ [ pkgs.zstd ]
                )
              }"
            '';
            # Static plugin metadata is already installed in dev/mkspecs/modules.
            # Merge its files before the upstream one-level mkspecs directory move.
            postInstall = ''
              moveToOutput "mkspecs/modules/*" "$dev"
            ''
            + (old.postInstall or "");
            # qtbase does not load its own setup hook. Supply its plugin prefix to
            # the inherited fixup so static consumers find the binary-output archives.
            postFixup = ''
              qtPluginPrefix=${lib.escapeShellArg prev.qtbase.qtPluginPrefix}
            ''
            + (old.postFixup or "");
          })
      );
      qttools = instrument (
        final.qtModule {
          pname = "qttools";
          propagatedBuildInputs = [ final.qtbase ];
          outputs = [
            "out"
            "dev"
          ];
          patches = [
            (pkgs.replaceVars
              "${pkgs.path}/pkgs/development/libraries/qt-5/modules/qttools-QT_HOST_DATA-refs.patch"
              {
                qtbaseDev = lib.getDev final.qtbase;
              }
            )
          ];
          postPatch = ''
            # No QDoc/LLVM, Designer plugins or QML module is used by these builds.
            printf '%s\n' \
              'TEMPLATE = subdirs' \
              'SUBDIRS = uiplugin uitools linguist' \
              'uiplugin.subdir = designer/src/uiplugin' \
              'uitools.subdir = designer/src/uitools' \
              'uitools.depends = uiplugin' \
              'linguist.depends = uitools' > src/src.pro
          '';
          devTools = [
            "bin/linguist"
            "bin/lconvert"
            "bin/lrelease"
            "bin/lrelease-pro"
            "bin/lupdate"
            "bin/lupdate-pro"
            "bin/lprodump"
          ];
          setupHook = "${pkgs.path}/pkgs/development/libraries/qt-5/hooks/qttools-setup-hook.sh";
        }
      );
    }
  );

  # Vendor SDKs only cross C interfaces. Discord's prebuilt ELF needs
  # libgcc_s.so.1 (not libstdc++.so.6); this runtime-only output is deliberately
  # separate from the selected compiler and never used to build C++ code.
  sdkRuntime = pkgs.gcc.cc.lib;
in
{
  inherit
    cc
    stdenv
    instrument
    gtest
    gbenchmark
    abseil-cpp
    icu
    re2
    libcpr
    cpptrace
    qt5
    sdkRuntime
    ;
}
