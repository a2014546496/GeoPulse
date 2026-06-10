# ── Test targets ────────────────────────────────────

# NMEA Parser tests
add_executable(test_nmea_parser test_nmea_parser.cpp)
target_link_libraries(test_nmea_parser PRIVATE gpsutils Qt6::Core Qt6::Test)
target_include_directories(test_nmea_parser PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/gpsutils)
add_test(NAME nmea_parser COMMAND test_nmea_parser)

# Geo Utils tests
add_executable(test_geo_utils test_geo_utils.cpp)
target_link_libraries(test_geo_utils PRIVATE gpsutils Qt6::Core Qt6::Test Qt6::Positioning)
target_include_directories(test_geo_utils PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/gpsutils)
add_test(NAME geo_utils COMMAND test_geo_utils)

# Config Manager tests
add_executable(test_config_manager test_config_manager.cpp)
target_link_libraries(test_config_manager PRIVATE gpsutils Qt6::Core Qt6::Test)
target_include_directories(test_config_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/gpsutils)
add_test(NAME config_manager COMMAND test_config_manager)

# Driver Factory tests
add_executable(test_driver_factory test_driver_factory.cpp)
target_link_libraries(test_driver_factory PRIVATE gpscomm Qt6::Core Qt6::Test Qt6::SerialPort Qt6::Network)
target_include_directories(test_driver_factory PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/gpscomm)
add_test(NAME driver_factory COMMAND test_driver_factory)
