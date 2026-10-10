add_library (star_platform_modules STATIC)
target_sources (star_platform_modules PUBLIC FILE_SET CXX_MODULES
    FILES StarP2PNetworkingService.cppm
          StarDesktopService.cppm
          StarStatisticsService.cppm
          StarUserGeneratedContentService.cppm)
set_target_properties (star_platform_modules PROPERTIES CXX_SCAN_FOR_MODULES ON)
target_include_directories (star_platform_modules PRIVATE
    ${STAR_EXTERN_INCLUDES}
    ${STAR_CORE_INCLUDES}
    ${STAR_PLATFORM_INCLUDES}
  )
target_link_libraries (star_platform_modules PUBLIC star_modules PRIVATE star_build_config)
