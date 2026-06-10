set(APP_SOURCES
    main.cpp
    geopulse_app.cpp
)

set(APP_HEADERS
    geopulse_app.h
)

set(APP_RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)

add_executable(GeoPulse ${APP_SOURCES} ${APP_HEADERS} ${APP_RESOURCES})

target_link_libraries(GeoPulse
    PRIVATE
        geopulse_core
        Qt6::Core
        Qt6::Quick
        Qt6::Qml
        Qt6::Positioning
        Qt6::Location
)

# Derive Qt6 install prefix from its CMake config location
# Qt6_DIR = <prefix>/lib/cmake/Qt6, so we go up 2 levels
get_filename_component(QT6_PREFIX "${Qt6_DIR}/../../.." ABSOLUTE)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/qt.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/qt.conf
    @ONLY
)

# Copy generated qt.conf and simulation data next to the binary
add_custom_command(TARGET GeoPulse POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/qt.conf
        $<TARGET_FILE_DIR:GeoPulse>/qt.conf
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/sim
        $<TARGET_FILE_DIR:GeoPulse>/sim
)
