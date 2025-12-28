cmake_minimum_required(VERSION 3.15)

find_package(Qt6 REQUIRED COMPONENTS Test)
find_package(GTest REQUIRED)
include(GoogleTest) 

# Core library for testing
add_library(flood_monitor_core STATIC
    ${CMAKE_SOURCE_DIR}/src/MonitoringData.cpp
    ${CMAKE_SOURCE_DIR}/src/Warning.cpp
    ${CMAKE_SOURCE_DIR}/src/WarningModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Station.cpp
    ${CMAKE_SOURCE_DIR}/src/StationModel.cpp
    ${CMAKE_SOURCE_DIR}/src/Measure.cpp
    ${CMAKE_SOURCE_DIR}/src/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/src/TypeUtils.cpp
    ${CMAKE_SOURCE_DIR}/include/WarningModel.hpp
    ${CMAKE_SOURCE_DIR}/include/StationModel.hpp
)

target_include_directories(flood_monitor_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(flood_monitor_core
    PRIVATE
        CURL::libcurl
        simdjson::simdjson
        Qt6::Core
        Qt6::Positioning
)

if(ENABLE_COVERAGE)
    target_compile_options(flood_monitor_core PRIVATE --coverage)
    target_link_options(flood_monitor_core PRIVATE --coverage)
endif()

# GoogleTest tests
add_executable(gtest_unit_tests
    unit/MonitoringDataTest.cpp
    unit/HttpClientTest.cpp
    unit/StationTest.cpp
    unit/MeasureTest.cpp
    unit/WarningTest.cpp
)

target_include_directories(gtest_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

target_link_libraries(gtest_unit_tests
    PRIVATE
        GTest::gtest_main
        GTest::gmock
        flood_monitor_core
        CURL::libcurl
        simdjson::simdjson 
)

# QtTest tests
add_executable(qttest_warning_model unit/WarningModelTest.cpp)
target_link_libraries(qttest_warning_model 
    PRIVATE 
        flood_monitor_core 
        Qt6::Test
        Qt6::Positioning
        CURL::libcurl
        simdjson::simdjson 
)

add_executable(qttest_station_model unit/StationModelTest.cpp)
target_link_libraries(qttest_station_model 
    PRIVATE 
        flood_monitor_core 
        Qt6::Test
        CURL::libcurl
        simdjson::simdjson 
)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt 
        HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET qttest_warning_model POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-compiler-runtime
                --debug
                "$<TARGET_FILE:qttest_warning_model>"
            COMMENT "Deploying Qt for qttest_warning_model..."
        )
        add_custom_command(TARGET qttest_station_model POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-compiler-runtime
                --debug
                "$<TARGET_FILE:qttest_station_model>"
            COMMENT "Deploying Qt for qttest_station_model..."
        )
    endif()
endif()

if(ENABLE_COVERAGE)
    target_compile_options(gtest_unit_tests PRIVATE --coverage)
    target_link_options(gtest_unit_tests PRIVATE --coverage)
    target_compile_options(qttest_warning_model PRIVATE --coverage)
    target_link_options(qttest_warning_model PRIVATE --coverage)
    target_compile_options(qttest_station_model PRIVATE --coverage)
    target_link_options(qttest_station_model PRIVATE --coverage)
endif()

gtest_discover_tests(gtest_unit_tests)
add_test(NAME qttest_warning_model COMMAND qttest_warning_model)
add_test(NAME qttest_station_model COMMAND qttest_station_model)

# Target for all unit tests
add_custom_target(unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
    DEPENDS gtest_unit_tests qttest_warning_model qttest_station_model
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all unit tests"
)