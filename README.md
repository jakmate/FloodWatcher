# FloodWatcher

[![Lint & Format](https://github.com/jakmate/FloodWatcher/actions/workflows/lint.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/lint.yml)
[![codecov](https://codecov.io/gh/jakmate/FloodWatcher/branch/main/graph/badge.svg)](https://codecov.io/gh/jakmate/FloodWatcher)
<!--
[![Build Linux](https://github.com/jakmate/FloodWatcher/actions/workflows/build-linux.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/build-linux.yml)
[![Build Windows](https://github.com/jakmate/FloodWatcher/actions/workflows/build-windows.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/build-windows.yml)
[![Build macOS](https://github.com/jakmate/FloodWatcher/actions/workflows/build-macos.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/build-macos.yml)
-->

Real-time flood warning monitor for the UK using Environment Agency data. This will updated progressively.
Features an interactive map dashboard built with Qt that displays flood warnings, monitoring stations, and water level measurements.

## To Do

- Improve qt (create clusters with stations?)
- Fix builds (Do once package manager sorted)
- Consider curl multi for polygons (was slower but try again)
- gtest vs Qt Test (or keep both use depending on file)
- API doc lies and there is an exception in northing and easting with double values not int

## Requirements

- C++17 or later
- CMake 3.15+
- libcurl
- simdjson
- Qt 6 (with Core, Qml, Gui, and Positioning modules)

## Building

```bash
git clone https://github.com/jakmate/FloodWatcher.git
cd FloodWatcher
cmake -GNinja -B build
cmake --build build
```

### With Conan

```bash
conan install . --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

## Testing

```bash
# Download gcovr
cmake -GNinja -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON -B build
cmake --build build --target unit_tests
cmake --build build --target coverage
```

## Profiling

```bash
cmake -GNinja -DENABLE_PROFILING=ON -B build
cmake --build build
cd build
./flood_monitor.exe
gprof flood_monitor.exe gmon.out > analysis.txt
# or for ease of read
gprof flood_monitor.exe gmon.out --flat-profile | head -30
```

## API Reference

Data source: UK Environment Agency Flood Monitoring API [[1](https://environment.data.gov.uk/flood-monitoring/doc/reference)]


## When using Conan for Qt:
### using msvc
"Qt6::QGeoPositionInfoSourceFactoryGeoclue2")
CMake Error at build/Release/generators/cmakedeps_macros.cmake:81 (message):
  Library 'qtposition_geoclue2' not found in package.  If
  'qtposition_geoclue2' is a system library, declare it with
  'cpp_info.system_libs' property
Call Stack (most recent call first):
  build/Release/generators/Qt6-Target-release.cmake:23 (conan_package_library_targets)
  build/Release/generators/Qt6Targets.cmake:124 (include)
  build/Release/generators/Qt6Config.cmake:16 (include)
  CMakeLists.txt:19 (find_package)
### using gcc with ucrt
does not compile fully...