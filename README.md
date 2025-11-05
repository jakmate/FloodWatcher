# FloodWatcher

Real-time flood warning monitor for the UK using Environment Agency data. This will updated progressively.
Plan it to make a dashboard using Qt and real time fetching from the API.

## Requirements

- C++17 or later
- CMake 3.15+
- libcurl
- nlohmann/json 3.2.0+

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## API Reference

Data source: UK Environment Agency Flood Monitoring API [[1](https://environment.data.gov.uk/flood-monitoring/doc/reference)]
