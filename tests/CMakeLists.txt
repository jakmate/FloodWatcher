cmake_minimum_required(VERSION 3.15)

# Add C++ tests
add_subdirectory(cpp)

# Add QML tests
#find_package(Qt6 COMPONENTS QuickTest)
#if(Qt6QuickTest_FOUND)
#    add_subdirectory(qml)
#endif()