cmake_minimum_required(VERSION 3.15)
project(FloodMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

# Find required packages
find_package(CURL REQUIRED)
find_package(simdjson REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Positioning)

# Create executable
add_executable(flood_monitor
    src/main.cpp
    src/MonitoringData.cpp
    src/Warning.cpp
    src/WarningModel.cpp
    src/Station.cpp
    src/StationModel.cpp
    src/Measure.cpp
    src/HttpClient.cpp
    src/TypeUtils.cpp
    include/MonitoringData.hpp
    include/Warning.hpp
    include/WarningModel.hpp
    include/Station.hpp
    include/StationModel.hpp
    include/Measure.hpp
    include/HttpClient.hpp
    include/GeometryTypes.hpp
    include/ThreadPool.hpp
    include/TypeUtils.hpp
    qml.qrc
)

# Target properties
target_compile_features(flood_monitor PRIVATE cxx_std_17)
target_include_directories(flood_monitor PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries
target_link_libraries(flood_monitor
    PRIVATE
        CURL::libcurl
        simdjson::simdjson
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Positioning
)

# Compiler warnings
if(MSVC)
    target_compile_options(flood_monitor PRIVATE /W4)
else()
    target_compile_options(flood_monitor PRIVATE -Wall -Wextra -pedantic)
endif()

# Coverage option
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
    message(STATUS "Building with coverage flags")
    add_library(coverage_flags INTERFACE)
    target_compile_options(coverage_flags INTERFACE --coverage -O0 -g)
    target_link_options(coverage_flags INTERFACE --coverage)
    target_link_libraries(flood_monitor PRIVATE coverage_flags)
endif()

# Profiling option
option(ENABLE_PROFILING "Enable profiling" OFF)
if(ENABLE_PROFILING)
    message(STATUS "Building with profiling flags")
    target_compile_definitions(flood_monitor PRIVATE ENABLE_PROFILING_EXIT)
    if(WIN32 AND MINGW)
        target_compile_options(flood_monitor PRIVATE -O0 -pg -g -no-pie)
        target_link_options(flood_monitor PRIVATE -pg -no-pie)
    elseif(UNIX)
        target_compile_options(flood_monitor PRIVATE -O0 -g -fno-omit-frame-pointer)
    endif()
endif()

# Testing
enable_testing()
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    message(STATUS "Building with tests")
    add_subdirectory(tests)
endif()

# Coverage report target
if(ENABLE_COVERAGE)
    find_program(GCOVR gcovr)
    if(GCOVR)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E remove_directory coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory coverage
            COMMAND ${CMAKE_CTEST_COMMAND} -j --output-on-failure
            COMMAND ${GCOVR}
                -j
                -r ${CMAKE_SOURCE_DIR}
                --filter ${CMAKE_SOURCE_DIR}/src/
                --filter ${CMAKE_SOURCE_DIR}/include/
                --exclude-unreachable-branches
                --html --html-details
                --print-summary
                -o coverage/index.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating filtered coverage report with gcovr"
        )
    else()
        message(WARNING "gcovr not found — please install gcovr to enable coverage reporting")
    endif()
endif()