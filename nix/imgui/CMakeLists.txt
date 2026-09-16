cmake_minimum_required(VERSION 3.23)
project(imgui VERSION 1.92.8 LANGUAGES CXX)

# Match the source set and features in the OpenStarbound vcpkg baseline
# d015e31e90838a4c9dfa3eed45979bc70d9357fc, without vcpkg's FreeType target name.
find_package(SDL3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(imgui STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    misc/cpp/imgui_stdlib.cpp
    misc/freetype/imgui_freetype.cpp
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_opengl3.cpp
)
add_library(imgui::imgui ALIAS imgui)
target_compile_features(imgui PUBLIC cxx_std_11)
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(imgui PUBLIC SDL3::SDL3 Freetype::Freetype OpenGL::GL ${CMAKE_DL_LIBS})

install(TARGETS imgui EXPORT imgui-targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES
    imgui.h imconfig.h imgui_internal.h
    imstb_textedit.h imstb_rectpack.h imstb_truetype.h
    misc/cpp/imgui_stdlib.h misc/freetype/imgui_freetype.h
    backends/imgui_impl_sdl3.h backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3_loader.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT imgui-targets NAMESPACE imgui:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imgui)
configure_package_config_file(imgui-config.cmake.in imgui-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imgui)
write_basic_package_version_file(imgui-config-version.cmake COMPATIBILITY ExactVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/imgui-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/imgui-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imgui)
install(FILES LICENSE.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/imgui)
