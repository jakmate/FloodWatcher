#pragma once
#include "Station.hpp"
#include "Warning.hpp"
#include <simdjson.h>
#include <vector>

class MonitoringData {
    friend class MonitoringDataTest;

  public:
    void parseWarnings(const simdjson::dom::element& apiResponse);
    void parseStations(const simdjson::dom::element& apiResponse);
    void fetchAllPolygonsAsync();

    const std::vector<Warning>& getWarnings() const {
      return warnings;
    }
    const std::vector<Station>& getStations() const {
      return stations;
    }

  private:
    std::vector<Warning> warnings;
    std::vector<Station> stations;
};
