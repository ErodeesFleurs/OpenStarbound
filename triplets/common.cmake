# C++ APIs must stay in the executable: xclang's hidden, static libc++ runtime
# is private to each shared object. Only libraries with C ABIs may be shared.
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)
set(VCPKG_LOAD_VCVARS_ENV OFF)
set(VCPKG_ENV_PASSTHROUGH_UNTRACKED XCLANG_ROOT MSYS2_ROOT)
set(VCPKG_HASH_ADDITIONAL_FILES "${CMAKE_CURRENT_LIST_FILE}")
if(PORT STREQUAL "qt5-base")
    list(APPEND VCPKG_HASH_ADDITIONAL_FILES
        "${CMAKE_CURRENT_LIST_DIR}/../cmake/patches/qtbase-libcxx.patch")
endif()
if(NOT EXISTS "$ENV{XCLANG_ROOT}/lib/cmake/xclang/toolchain.cmake")
    message(FATAL_ERROR "Set XCLANG_ROOT to the unpacked xclang 23.1.2.9 toolchain")
endif()
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{XCLANG_ROOT}/lib/cmake/xclang/toolchain.cmake")
# Compiler targets reach both CMake ports and the autotools/Qt compiler probes.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
    "-DXCLANG_TARGET=${XCLANG_TARGET}"
    "-DCMAKE_C_COMPILER_TARGET=${XCLANG_TARGET}"
    "-DCMAKE_CXX_COMPILER_TARGET=${XCLANG_TARGET}")

# Qt5's qmake bootstrap must not auto-detect MSVC or select a GNU mkspec.
if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "MinGW")
    # MinGW is vcpkg's platform name; CMake's platform is Windows. The official
    # xclang file replaces vcpkg's mingw.cmake, which normally translates this.
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCMAKE_SYSTEM_NAME=Windows")
    set(VCPKG_QT_TARGET_MKSPEC win32-clang-g++)
    set(VCPKG_QT_HOST_MKSPEC win32-clang-g++)
    # vcpkg's Windows port environment is isolated from the invoking shell.
    if(DEFINED ENV{MSYS2_ROOT})
        set(ENV{PATH} "$ENV{XCLANG_ROOT}/bin;$ENV{MSYS2_ROOT}/ucrt64/bin;$ENV{PATH}")
    else()
        set(ENV{PATH} "$ENV{XCLANG_ROOT}/bin;$ENV{PATH}")
    endif()
    if(PORT STREQUAL "qt5-base")
        set(ENV{CC} "$ENV{XCLANG_ROOT}/bin/clang.exe")
        set(ENV{CXX} "$ENV{XCLANG_ROOT}/bin/clang++.exe")
        # The MinGW mkspec otherwise hardcodes GNU ar for static libraries.
        set(ENV{QMAKEFLAGS} "-late \"QMAKE_LIB=llvm-ar -rc\" \"QMAKE_AR=llvm-ar -rc\" \"QMAKE_OBJCOPY=llvm-objcopy\"")
    endif()
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(VCPKG_QT_TARGET_MKSPEC linux-clang)
    set(VCPKG_QT_HOST_MKSPEC linux-clang)
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(VCPKG_QT_TARGET_MKSPEC macx-clang)
    set(VCPKG_QT_HOST_MKSPEC macx-clang)
    set(VCPKG_OSX_DEPLOYMENT_TARGET 14.0)
endif()
if(PORT MATCHES "^discord-")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "MinGW" AND PORT STREQUAL "sdl3")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux" AND PORT STREQUAL "jemalloc")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

if(PORT STREQUAL "opus")
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
        "-DOPUS_INSTALL_PKG_CONFIG_MODULE=OFF"
        "-DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF"
        "-DOPUS_ENABLE_FLOAT_API=ON"
        "-DOPUS_FLOAT_APPROX=ON"
        "-DOPUS_STACK_PROTECTOR=OFF"
        "-DOPUS_NONTHREADSAFE_PSEUDOSTACK=OFF"
        "-DOPUS_USE_ALLOCA=ON"
        "-DBUILD_TESTING=OFF")
    if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
        list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
            "-DOPUS_X86_MAY_HAVE_SSE=ON"
            "-DOPUS_X86_MAY_HAVE_AVX=ON"
            "-DOPUS_X86_MAY_HAVE_SSE4_1=ON")
    else()
        list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
            "-DOPUS_ARM_MAY_HAVE_NEON=ON"
            "-DOPUS_ARM_MAY_HAVE_NEON_INTR=ON")
    endif()
endif()
