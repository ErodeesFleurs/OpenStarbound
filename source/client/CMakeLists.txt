set(star_client_HEADERS
      StarClientApplication.hpp
      StarRenderingLuaBindings.hpp
    )

set(star_client_SOURCES
      StarClientApplication.cpp
      StarRenderingLuaBindings.cpp
    )

if(STAR_SYSTEM_WINDOWS)
  set(star_client_RESOURCES
      starbound.rc
    )
endif()

add_executable(starbound WIN32
  $<TARGET_OBJECTS:star_extern> $<TARGET_OBJECTS:star_core> $<TARGET_OBJECTS:star_base> $<TARGET_OBJECTS:star_game>
  $<TARGET_OBJECTS:star_application> $<TARGET_OBJECTS:star_rendering> $<TARGET_OBJECTS:star_windowing> $<TARGET_OBJECTS:star_frontend>
  ${star_client_HEADERS} ${star_client_SOURCES} ${star_client_RESOURCES})
target_compile_features(starbound PRIVATE cxx_std_20)
target_include_directories(starbound PRIVATE
    ${STAR_FRONTEND_INCLUDES}
    ${STAR_WINDOWING_INCLUDES}
    ${STAR_RENDERING_INCLUDES}
    ${STAR_APPLICATION_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
    ${STAR_GAME_INCLUDES}
    ${STAR_BASE_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_EXTERN_INCLUDES})

if(STAR_PRECOMPILED_HEADERS)
  target_precompile_headers(starbound REUSE_FROM star_core)
endif()

if(STAR_SYSTEM_MACOS)
  set_target_properties(starbound PROPERTIES LINK_FLAGS "-Wl,-rpath,'@executable_path'")
elseif(UNIX)
  set_target_properties(starbound PROPERTIES LINK_FLAGS "-Wl,--export-dynamic,-rpath,'$ORIGIN'")
endif()

if(STAR_SYSTEM_MACOS)
  set_target_properties(starbound PROPERTIES XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
endif()

target_link_libraries(starbound PRIVATE star_build_definitions star_external_libraries star_external_gui_libraries)
