cmake_minimum_required(VERSION 3.15)
project(FloodMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Try Qt6 first, fallback to Qt5
find_package(Qt6 COMPONENTS Gui Qml Positioning QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Quick Gui Qml Location)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Using Qt5")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/FloodMonitoringData.cpp
    src/FloodWarning.cpp
    src/FloodWarningModel.cpp
    src/Station.cpp
    src/StationModel.cpp
    src/Measure.cpp
    src/HttpClient.cpp
    src/GeometryTypes.cpp
)

# Header files
set(HEADERS
    include/FloodMonitoringData.hpp
    include/FloodWarning.hpp
    include/FloodWarningModel.hpp
    include/Station.hpp
    include/StationModel.hpp
    include/Measure.hpp
    include/HttpClient.hpp
    include/GeometryTypes.hpp
)

# Create executable
add_executable(flood_monitor
    ${SOURCES}
    ${HEADERS}
    qml.qrc
)

# Include directories
target_include_directories(flood_monitor PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
    set(CURL_STATICLIB ON)
    target_compile_definitions(flood_monitor PRIVATE CURL_STATICLIB)
endif()

# Link libraries
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(flood_monitor
        PRIVATE
            CURL::libcurl
            nlohmann_json::nlohmann_json
            Qt6::Gui
            Qt6::Qml
            Qt6::Positioning
    )
else()
    target_link_libraries(flood_monitor
        PRIVATE
            CURL::libcurl
            nlohmann_json::nlohmann_json
            Qt5::Quick
            Qt5::Gui
            Qt5::Qml
            Qt5::Location
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(flood_monitor PRIVATE /W4)
else()
    target_compile_options(flood_monitor PRIVATE -Wall -Wextra -pedantic)
endif()