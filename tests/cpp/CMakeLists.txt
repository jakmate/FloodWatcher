cmake_minimum_required(VERSION 3.15)

find_package(Qt6 REQUIRED COMPONENTS Test)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

# Include the GoogleTest module
include(GoogleTest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# First, create a library from core components
# This is needed because you can't link an executable to an executable
add_library(flood_monitor_core STATIC
    ${CMAKE_SOURCE_DIR}/src/MonitoringData.cpp
    ${CMAKE_SOURCE_DIR}/src/Warning.cpp
    ${CMAKE_SOURCE_DIR}/src/WarningModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Station.cpp
    ${CMAKE_SOURCE_DIR}/src/StationModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Measure.cpp
    ${CMAKE_SOURCE_DIR}/src/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/include/WarningModel.hpp
    #${CMAKE_SOURCE_DIR}/include/StationModel.hpp
)

if(ENABLE_COVERAGE)
    target_compile_options(flood_monitor_core PRIVATE --coverage)
    target_link_options(flood_monitor_core PRIVATE --coverage)
endif()

target_include_directories(flood_monitor_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(flood_monitor_core
    PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Qt6::Core
        Qt6::Positioning
)

# Create test executables for GoogleTest
add_executable(gtest_unit_tests
    unit/MonitoringDataTest.cpp
    unit/HttpClientTest.cpp
    unit/StationTest.cpp
    unit/MeasureTest.cpp
    unit/WarningTest.cpp
)

target_link_libraries(gtest_unit_tests
    PRIVATE
        GTest::gtest_main
        GTest::gmock
        flood_monitor_core
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Qt6::Core
        Qt6::Positioning
)

# Register GoogleTest tests
gtest_discover_tests(gtest_unit_tests)

# Create test executables for QtTest
add_executable(qttest_unit_tests
    unit/WarningModelTest.cpp
    #unit/StationModelTest.cpp
)

target_link_libraries(qttest_unit_tests
    PRIVATE
        flood_monitor_core
        Qt6::Core
        Qt6::Positioning
        Qt6::Test
)

# Register QtTest tests
add_test(NAME qttest_unit_tests COMMAND qttest_unit_tests)
