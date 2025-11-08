cmake_minimum_required(VERSION 3.15)
project(FloodMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Location)

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

# Link libraries
target_link_libraries(flood_monitor
    PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Location
)

# Compiler warnings
if(MSVC)
    target_compile_options(flood_monitor PRIVATE /W4)
else()
    target_compile_options(flood_monitor PRIVATE -Wall -Wextra -pedantic)
endif()