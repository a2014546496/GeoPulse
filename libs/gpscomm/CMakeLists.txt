set(GPSCOMM_SOURCES
    serial_driver.cpp
    tcp_driver.cpp
    udp_driver.cpp
    driver_factory.cpp
)

set(GPSCOMM_HEADERS
    gpscomm_global.h
    igps_driver.h
    serial_driver.h
    tcp_driver.h
    udp_driver.h
    driver_factory.h
)

add_library(gpscomm STATIC ${GPSCOMM_SOURCES} ${GPSCOMM_HEADERS})

target_include_directories(gpscomm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gpscomm
    PUBLIC
        Qt6::Core
        Qt6::SerialPort
        Qt6::Network
)

target_compile_definitions(gpscomm PRIVATE GPSCOMM_LIBRARY)
