cmake_minimum_required(VERSION 3.15)
project(FloodMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Source files
add_executable(flood_monitor
    main.cpp
    FloodMonitoringData.cpp
    FloodWarning.cpp
    Station.cpp
    Measure.cpp
    HttpClient.cpp
    GeometryTypes.cpp
)

# Link libraries
target_link_libraries(flood_monitor
    PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

# Compiler warnings
if(MSVC)
    target_compile_options(flood_monitor PRIVATE /W4)
else()
    target_compile_options(flood_monitor PRIVATE -Wall -Wextra -pedantic)
endif()