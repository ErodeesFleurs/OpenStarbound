SET (star_extern_HEADERS
    curve25519/include/curve25519_dh.h
    curve25519/include/ed25519_signature.h
    curve25519/include/external_calls.h
    imgui_iterator.inl
    imgui_lua_bindings.hpp
    lauxlib.h
    lua.h
    lua.hpp
    luaconf.h
    lualib.h

    xxhash.h
  )

SET (star_extern_SOURCES
    curve25519/source/sha512.c
    curve25519/source/curve25519_dh.c
    curve25519/source/curve25519_mehdi.c
    curve25519/source/curve25519_order.c
    curve25519/source/curve25519_utils.c
    curve25519/source/custom_blind.c
    curve25519/source/ed25519_sign.c
    curve25519/source/ed25519_verify.c

    xxhash.c

    imgui_lua_bindings.cpp
    lua/lapi.c
    lua/lauxlib.c
    lua/lbaselib.c
    lua/lbitlib.c
    lua/lcode.c
    lua/lcorolib.c
    lua/lctype.c
    lua/ldblib.c
    lua/ldebug.c
    lua/ldo.c
    lua/ldump.c
    lua/lfunc.c
    lua/lgc.c
    lua/linit.c
    lua/liolib.c
    lua/llex.c
    lua/lmathlib.c
    lua/lmem.c
    lua/loadlib.c
    lua/lobject.c
    lua/lopcodes.c
    lua/loslib.c
    lua/lparser.c
    lua/lstate.c
    lua/lstring.c
    lua/lstrlib.c
    lua/ltable.c
    lua/ltablib.c
    lua/ltm.c
    lua/lundump.c
    lua/lutf8lib.c
    lua/lvm.c
    lua/lzio.c
  )

ADD_LIBRARY(star_extern OBJECT ${star_extern_SOURCES} ${star_extern_HEADERS})

TARGET_INCLUDE_DIRECTORIES(star_extern PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/curve25519/include
)

SET_TARGET_PROPERTIES(star_extern PROPERTIES C_VISIBILITY_PRESET hidden)

# CTRE

SET(CTRE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(CTRE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY(ctre EXCLUDE_FROM_ALL)

IF (TARGET ctre)
    ADD_LIBRARY(ctre_module STATIC)
    TARGET_LINK_LIBRARIES(ctre_module PUBLIC ctre)
    TARGET_SOURCES(ctre_module
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ctre/
        FILES ctre/ctre.cppm
    )
    list(APPEND LOCAL_EXT_LIBS ctre_module)
ENDIF()

# --- RE2 ---

FIND_PACKAGE(re2 REQUIRED)
IF (TARGET re2::re2)
    ADD_LIBRARY(re2_module STATIC)
    TARGET_SOURCES(re2_module
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES re2.cppm
    )
    TARGET_LINK_LIBRARIES(re2_module PUBLIC re2::re2)
    LIST(APPEND LOCAL_EXT_LIBS re2_module)
ENDIF()

SET(STAR_EXT_LIBS ${STAR_EXT_LIBS} star_extern ${LOCAL_EXT_LIBS} PARENT_SCOPE)
