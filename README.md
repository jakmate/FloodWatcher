# FloodWatcher

[![Lint & Format](https://github.com/jakmate/FloodWatcher/actions/workflows/lint.yml/badge.svg)](https://github.com/jakmate/FloodWatcher/actions/workflows/lint.yml)
[![Build Linux](https://github.com/jakmate/FloodWatcher/actions/workflows/build.yml/badge.svg?branch=main&event=push&job=build-linux)](https://github.com/jakmate/FloodWatcher/actions/workflows/build.yml)
[![Build Windows](https://github.com/jakmate/FloodWatcher/actions/workflows/build.yml/badge.svg?branch=main&event=push&job=build-windows)](https://github.com/jakmate/FloodWatcher/actions/workflows/build.yml)
[![Build macOS](https://github.com/jakmate/FloodWatcher/actions/workflows/build.yml/badge.svg?branch=main&event=push&job=build-macos)](https://github.com/jakmate/FloodWatcher/actions/workflows/build.yml)

Real-time flood warning monitor for the UK using Environment Agency data. This will updated progressively.
Features an interactive map dashboard built with Qt that displays flood warnings, monitoring stations, and water level measurements.

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
