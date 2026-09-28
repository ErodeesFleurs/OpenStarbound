include($ENV{VCPKG_ROOT}/scripts/toolchains/linux.cmake)

if(DEFINED ENV{LLVM_PATH})
  set(CMAKE_C_COMPILER "$ENV{LLVM_PATH}/bin/clang")
  set(CMAKE_CXX_COMPILER "$ENV{LLVM_PATH}/bin/clang++")
else()
  find_program(CMAKE_C_COMPILER NAMES clang REQUIRED)
  find_program(CMAKE_CXX_COMPILER NAMES clang++ REQUIRED)
endif()