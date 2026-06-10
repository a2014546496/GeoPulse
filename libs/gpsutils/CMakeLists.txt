set(GPSUTILS_SOURCES
    nmea_parser.cpp
    config_manager.cpp
    logger.cpp
    geo_utils.cpp
)

set(GPSUTILS_HEADERS
    nmea_parser.h
    config_manager.h
    logger.h
    geo_utils.h
)

add_library(gpsutils STATIC ${GPSUTILS_SOURCES} ${GPSUTILS_HEADERS})

target_include_directories(gpsutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gpsutils
    PUBLIC
        Qt6::Core
        Qt6::Positioning
)
