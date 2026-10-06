set (star_client_HEADERS
      StarClientApplication.hpp
    )

set (star_client_SOURCES
      StarClientApplication.cpp
      StarRenderingLuaBindings.cpp
    )

if (STAR_SYSTEM_WINDOWS)
  set (star_client_RESOURCES
      starbound.rc
      starbound.exe.manifest
    )
endif ()

add_executable (starbound WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  $<TARGET_OBJECTS:star_application> $<TARGET_OBJECTS:star_rendering> $<TARGET_OBJECTS:star_frontend>
  ${star_client_HEADERS} ${star_client_SOURCES} ${star_client_RESOURCES})
target_include_directories (starbound PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
    ${STAR_WINDOWING_INCLUDES}
    ${STAR_FRONTEND_INCLUDES}
  )
add_library (star_client_module_interfaces OBJECT)
target_sources (star_client_module_interfaces PUBLIC FILE_SET CXX_MODULES FILES
    StarRenderingLuaBindings.cppm
  )
target_link_libraries (star_client_module_interfaces PRIVATE star_build_config)
set_source_files_properties (StarRenderingLuaBindings.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if (STAR_PRECOMPILED_HEADERS)
  target_precompile_headers (starbound REUSE_FROM star_core)
endif()

if(STAR_SYSTEM_MACOS)
  target_link_options (starbound PRIVATE "LINKER:-rpath,@executable_path")
elseif (UNIX)
  target_link_options (starbound PRIVATE "LINKER:--export-dynamic" "LINKER:-rpath,$ORIGIN")
endif()

if (STAR_SYSTEM_MACOS)
  set_target_properties (starbound PROPERTIES XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
endif()

target_link_libraries (starbound PRIVATE
  star_client_module_interfaces
  star_base_modules star_frontend_modules star_build_config star_client_dependencies
  $<LINK_ONLY:star_core_dependencies> $<LINK_ONLY:star_extern_dependencies>
  $<LINK_ONLY:star_application_dependencies> $<LINK_ONLY:star_voice_dependencies>)
set_target_properties (starbound PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(DEFINED STAR_MSVC_RUNTIME_LIBRARY)
  set_target_properties (star_client_module_interfaces starbound PROPERTIES MSVC_RUNTIME_LIBRARY "${STAR_MSVC_RUNTIME_LIBRARY}")
endif()
