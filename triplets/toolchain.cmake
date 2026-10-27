include("$ENV{XCLANG_ROOT}/lib/cmake/xclang/toolchain.cmake")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux"
        AND NOT OSB_COMPILER_ROOT
        AND DEFINED ENV{OSB_NATIVE_COMPILER_ROOT}
        AND NOT "$ENV{OSB_NATIVE_COMPILER_ROOT}" STREQUAL "")
    cmake_host_system_information(RESULT OSB_HOST_PROCESSOR QUERY OS_PLATFORM)
    if(OSB_HOST_PROCESSOR MATCHES "^(arm64|ARM64)$")
        set(OSB_HOST_PROCESSOR aarch64)
    endif()
    if(NOT XCLANG_TARGET OR XCLANG_TARGET STREQUAL "${OSB_HOST_PROCESSOR}-unknown-linux-gnu")
        set(OSB_COMPILER_ROOT "$ENV{OSB_NATIVE_COMPILER_ROOT}")
    endif()
endif()

# A native wrapper may provide current C system headers while retaining the
# same xclang compiler and bundled static C++ runtime as the cross target.
if(OSB_COMPILER_ROOT)
    list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES OSB_COMPILER_ROOT)
    if(CMAKE_HOST_WIN32)
        set(OSB_COMPILER_SUFFIX ".exe")
    else()
        set(OSB_COMPILER_SUFFIX "")
    endif()
    set(CMAKE_C_COMPILER "${OSB_COMPILER_ROOT}/bin/clang${OSB_COMPILER_SUFFIX}")
    set(CMAKE_CXX_COMPILER "${OSB_COMPILER_ROOT}/bin/clang++${OSB_COMPILER_SUFFIX}")
    set(CMAKE_ASM_COMPILER "${OSB_COMPILER_ROOT}/bin/clang${OSB_COMPILER_SUFFIX}")
    # Nix's scanner wrapper supplies the same native SDK flags as its compiler.
    set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS "${OSB_COMPILER_ROOT}/bin/clang-scan-deps${OSB_COMPILER_SUFFIX}")
endif()

# xclang's Darwin driver defaults to ld64.lld, but CMake's Apple binutils
# discovery searches for ld. vcpkg exports CMAKE_LINKER to Meson's c_ld/cpp_ld;
# keep that override on the bundled linker instead of selecting Apple's ld.
if(XCLANG_TARGET MATCHES "-apple-darwin$" OR (NOT XCLANG_TARGET AND CMAKE_HOST_APPLE))
    set(CMAKE_LINKER "${XCLANG_ROOT}/bin/ld64.lld")
    if(CMAKE_HOST_WIN32)
        string(APPEND CMAKE_LINKER ".exe")
    endif()
endif()
