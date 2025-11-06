#pragma once
#include <nlohmann/json.hpp>
#include "FloodWarning.hpp"
#include "Station.hpp"

using json = nlohmann::json;

class FloodMonitoringData {
public:
    void parseFloodWarnings(const json& apiResponse);
    void parseStations(const json& apiResponse);
    
    const std::vector<FloodWarning>& getFloodWarnings() const { return floodWarnings; }
    const std::vector<Station>& getStations() const { return stations; }

private:
    std::vector<FloodWarning> floodWarnings;
    std::vector<Station> stations;
};