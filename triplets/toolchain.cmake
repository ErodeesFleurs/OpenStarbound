include("$ENV{XCLANG_ROOT}/lib/cmake/xclang/toolchain.cmake")

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
endif()
