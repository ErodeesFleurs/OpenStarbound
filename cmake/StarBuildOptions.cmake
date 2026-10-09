# Add project defaults only when the caller has not selected that class of
# option. CMake's own configuration initializers are not caller overrides.
function(star_default_compile_options config kind)
  string(TOUPPER "${config}" config_upper)
  if(kind STREQUAL "optimization")
    set(override_pattern "(^|[ \t])-O[^ \t]*([ \t]|$)")
  elseif(kind STREQUAL "math")
    set(override_pattern "(^|[ \t])-f(no-)?fast-math([ \t]|$)")
  else()
    set(override_pattern "(^|[ \t])-g([0-9]+|line-tables-only)?([ \t]|$)")
  endif()
  foreach(language C CXX)
    set(flag_variable CMAKE_${language}_FLAGS_${config_upper})
    string(STRIP "${${flag_variable}}" config_flags)
    string(STRIP "${${flag_variable}_INIT}" initial_flags)
    set(caller_flags "${CMAKE_${language}_FLAGS}")
    if(STAR_EXPLICIT_${language}_FLAGS_${config_upper} OR NOT config_flags STREQUAL initial_flags)
      string(APPEND caller_flags " ${config_flags}")
    endif()
    if(NOT caller_flags MATCHES "${override_pattern}")
      foreach(option IN LISTS ARGN)
        target_compile_options(star_build_config INTERFACE
          "$<$<AND:$<COMPILE_LANGUAGE:${language}>,$<CONFIG:${config}>>:${option}>")
      endforeach()
    endif()
  endforeach()
endfunction()

target_compile_definitions(star_build_config INTERFACE
  "$<$<CONFIG:Release,RelWithDebInfo>:NDEBUG>")

target_compile_options(star_build_config INTERFACE
  "$<$<COMPILE_LANGUAGE:C,CXX>:-Wall;-Wextra;-Wuninitialized;-Wno-parentheses-equality;-Wno-deprecated-declarations;-Wno-nan-infinity-disabled>"
  "$<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>")
target_link_options(star_build_config INTERFACE
  "$<$<LINK_LANGUAGE:CXX>:-stdlib=libc++>")
if(STAR_SYSTEM_MACOS)
  target_link_options(star_build_config INTERFACE "LINKER:-export_dynamic")
elseif(NOT STAR_SYSTEM_WINDOWS)
  target_link_options(star_build_config INTERFACE "LINKER:--export-dynamic")
endif()

if(NOT STAR_SYSTEM_WINDOWS AND NOT STAR_SYSTEM_MACOS)
  target_compile_options(star_build_config INTERFACE "$<$<COMPILE_LANGUAGE:C,CXX>:-pthread>")
  target_compile_definitions(star_build_config INTERFACE _REENTRANT)
  target_link_options(star_build_config INTERFACE -pthread)
endif()

foreach(config RelWithAsserts RelWithDebInfo Release)
  star_default_compile_options(${config} optimization -O3)
  star_default_compile_options(${config} math -ffast-math)
endforeach()

foreach(config Debug RelWithAsserts RelWithDebInfo)
  if(STAR_DEBUG_SYMBOL_LEVEL STREQUAL "minimal" OR
      (STAR_DEBUG_SYMBOL_LEVEL STREQUAL "default" AND config STREQUAL "RelWithDebInfo"))
    set(debug_option -gline-tables-only)
  else()
    set(debug_option -g)
  endif()
  star_default_compile_options(${config} debug ${debug_option})
endforeach()
if(NOT STAR_SYSTEM_WINDOWS)
  target_compile_options(star_build_config INTERFACE
    "$<$<AND:$<COMPILE_LANGUAGE:C,CXX>,$<CONFIG:RelWithDebInfo>>:-gz=zlib>")
endif()

if(STAR_SYSTEM_MACOS)
  target_link_options(star_build_config INTERFACE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:-flat_namespace;-undefined;suppress>")
endif()
