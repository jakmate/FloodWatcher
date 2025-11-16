#pragma once
#include "FloodWarning.hpp"
#include "Station.hpp"
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

class FloodMonitoringData {
  public:
    void parseFloodWarnings(const json& apiResponse);
    void parseStations(const json& apiResponse);
    void fetchAllPolygonsAsync();

    const std::vector<FloodWarning>& getFloodWarnings() const {
      return floodWarnings;
    }
    const std::vector<Station>& getStations() const {
      return stations;
    }

  private:
    std::vector<FloodWarning> floodWarnings;
    std::vector<Station> stations;
};
