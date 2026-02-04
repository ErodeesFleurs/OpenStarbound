SET (star_platform_HEADERS
    StarP2PNetworkingService.hpp
    StarStatisticsService.hpp
    StarUserGeneratedContentService.hpp
)

ADD_LIBRARY (star_platform OBJECT ${star_platform_HEADERS})

TARGET_INCLUDE_DIRECTORIES(star_platform PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/extern
)

TARGET_LINK_LIBRARIES(star_platform PUBLIC
    star_core
    star_extern
)

IF(STAR_PRECOMPILED_HEADERS)
    TARGET_PRECOMPILE_HEADERS (star_platform REUSE_FROM star_core)
ENDIF()
