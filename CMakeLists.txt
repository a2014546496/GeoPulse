cmake_minimum_required(VERSION 3.16)
project(GeoPulse
    VERSION 1.0.0
    DESCRIPTION "Real-time GPS monitoring and tracking desktop application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Qt6 dependencies ──────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    Qml
    Positioning
    Location
    SerialPort
    Network
)

# ── Subdirectories ────────────────────────────────────
add_subdirectory(libs/gpscomm)
add_subdirectory(libs/gpsutils)
add_subdirectory(src/core)
add_subdirectory(src/app)

# ── Tests (optional) ──────────────────────────────────
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()
