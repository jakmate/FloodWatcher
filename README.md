# FloodWatcher

Real-time flood warning monitor for the UK using Environment Agency data. This will updated progressively.
Plan it to make a dashboard using Qt and real time fetching from the API.

## Requirements

- C++17 or later
- CMake 3.15+
- libcurl
- nlohmann/json
- Qt

## Building

```bash
mkdir build && cd build
cmake -G Ninja ..
cmake --build .
```

## API Reference

Data source: UK Environment Agency Flood Monitoring API [[1](https://environment.data.gov.uk/flood-monitoring/doc/reference)]
