cmake_minimum_required(VERSION 3.15)
project(FloodMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)   

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Location)

# Source files
qt_add_executable(flood_monitor
    main.cpp
    FloodMonitoringData.cpp
    FloodWarning.cpp
    Station.cpp
    StationModel.cpp
    Measure.cpp
    HttpClient.cpp
    GeometryTypes.cpp
)

qt_add_qml_module(flood_monitor
    URI FloodMonitor
    VERSION 0.1
    QML_FILES
        main.qml
        MapArea.qml
        RightPanel.qml
)

# Link libraries
target_link_libraries(flood_monitor
    PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Qt::Location
)

set(qml_resource_files
    "main.qml"
    "MapArea.qml"
    "RightPanel.qml"
)

qt6_add_resources(flood_monitor "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

# Compiler warnings
if(MSVC)
    target_compile_options(flood_monitor PRIVATE /W4)
else()
    target_compile_options(flood_monitor PRIVATE -Wall -Wextra -pedantic)
endif()