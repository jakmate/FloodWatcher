# Minimum CMake version for gtest_discover_tests
cmake_minimum_required(VERSION 3.14)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
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
    ${CMAKE_SOURCE_DIR}/src/FloodMonitoringData.cpp
    ${CMAKE_SOURCE_DIR}/src/FloodWarning.cpp
    ${CMAKE_SOURCE_DIR}/src/FloodWarningModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Station.cpp
    ${CMAKE_SOURCE_DIR}/src/StationModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Measure.cpp
    ${CMAKE_SOURCE_DIR}/src/HttpClient.cpp
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
        Qt6::Positioning
)

# Create test executables
add_executable(unit_tests
    #unit/FloodMonitoringDataTest.cpp
    unit/HttpClientTest.cpp
    #unit/StationTest.cpp
    #unit/MeasureTest.cpp
    #unit/FloodWarningTest.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main
        GTest::gmock
        flood_monitor_core
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Qt6::Positioning
)

# Register tests using gtest_discover_tests
gtest_discover_tests(unit_tests)
