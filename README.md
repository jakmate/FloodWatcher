# FloodWatcher

[![Lint & Format](https://github.com/jakmate/FloodWatcher/actions/workflows/lint.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/lint.yml)
<!--
[![Build Linux](https://github.com/jakmate/FloodWatcher/actions/workflows/build-linux.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/build-linux.yml)
[![Build Windows](https://github.com/jakmate/FloodWatcher/actions/workflows/build-windows.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/build-windows.yml)
[![Build macOS](https://github.com/jakmate/FloodWatcher/actions/workflows/build-macos.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/build-macos.yml)
-->

Real-time flood warning monitor for the UK using Environment Agency data. This will updated progressively.
Features an interactive map dashboard built with Qt that displays flood warnings, monitoring stations, and water level measurements.

## To Do

- Remove redundant parsing for measurements
- Load warnings in bg on launch??? Maybe ignore severrity level 4?
- Animation in panel when warning clicked on map
- Highlight selected warning?
- Add qmlformat to .yml

## Requirements

- C++17 or later
- CMake 3.15+
- libcurl
- nlohmann/json
- Qt 6 (with Qml, Gui, and Positioning modules)

## Building

```bash
git clone https://github.com/jakmate/FloodWatcher.git
cd FloodWatcher
mkdir build && cd build
cmake -G Ninja ..
cmake --build .
```

## API Reference

Data source: UK Environment Agency Flood Monitoring API [[1](https://environment.data.gov.uk/flood-monitoring/doc/reference)]
