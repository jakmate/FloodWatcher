# gcovr does not work with msvc so either change compiler to clang/gcc or change coverage to something else

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

- Fix builds (Do once package manager sorted)
- Consider curl multi for polygons (was slower but try again)
- gtest vs Qt Test (or keep both use depending on file)
- API doc lies and there is an exception in northing and easting with double values not int

## Requirements

- C++17 or later
- CMake 3.15+
- libcurl 8.17.0
- simdjson 4.2.2
- Qt 6.10 (with Core, Qml, Gui, and Positioning modules)

### Test Requirements

- Google Test 1.17.0
- Qt Test

## Building

### Windows

Most likely will have to mess with CMakeUserPresets to make it ignore MSYS2 or mingw installs, if using msvc.

### Using system dependencies

```bash
git clone https://github.com/jakmate/FloodWatcher.git
cd FloodWatcher
cmake -GNinja -B build
cmake --build build
```

### Using Conan

```bash
# Download aqtinstall and donwload Qt
conan install . --build=missing
cmake --preset conan-debug
cmake --build --preset conan-debug
```

## Testing

### Using system dependencies

```bash
# Download gcovr
cmake -GNinja -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON -B build
cmake --build build --target unit_tests
cmake --build build --target coverage
```

### Using Conan

```bash
# Download gcovr
cmake --preset conan-debug -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON
cmake --build --preset conan-debug --target unit_tests
cmake --build --preset conan-debug --target coverage
```

## Profiling

### Using system dependencies

```bash
cmake --preset conan-debug -DENABLE_PROFILING=ON
cmake --build --preset conan-debug
cd build
./flood_monitor.exe
gprof flood_monitor.exe gmon.out > analysis.txt
# or for ease of read
gprof flood_monitor.exe gmon.out --flat-profile | head -30
```

### Using Conan

```bash
cmake --preset conan-debug -DENABLE_PROFILING=ON
cmake --build --preset conan-debug
cd build
./flood_monitor.exe
gprof flood_monitor.exe gmon.out > analysis.txt
# or for ease of read
gprof flood_monitor.exe gmon.out --flat-profile | head -30
```

## API Reference

Data source: UK Environment Agency Flood Monitoring API [[1](https://environment.data.gov.uk/flood-monitoring/doc/reference)]
