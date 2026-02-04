SET (star_extern_HEADERS
    curve25519/include/curve25519_dh.h
    curve25519/include/ed25519_signature.h
    curve25519/include/external_calls.h
    imgui_iterator.inl

    fast_float.h
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


IF (STAR_USE_RPMALLOC)
    LIST(APPEND star_extern_HEADERS rpmalloc.h rpnew.h)
    LIST(APPEND star_extern_SOURCES rpmalloc.c)
ENDIF()

ADD_LIBRARY(star_extern OBJECT ${star_extern_SOURCES} ${star_extern_HEADERS})

TARGET_INCLUDE_DIRECTORIES(star_extern PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/curve25519/include
)

SET_TARGET_PROPERTIES(star_extern PROPERTIES C_VISIBILITY_PRESET hidden)
