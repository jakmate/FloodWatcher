#pragma once
#include "Station.hpp"
#include "Warning.hpp"
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

class MonitoringData {
    friend class MonitoringDataTest;

  public:
    void parseWarnings(const json& apiResponse);
    void parseStations(const json& apiResponse);
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
