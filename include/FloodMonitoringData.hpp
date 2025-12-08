#pragma once
#include "FloodWarning.hpp"
#include "HttpClientAdapter.hpp"
#include "IHttpClient.hpp"
#include "Station.hpp"
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

class FloodMonitoringData {
    friend class FloodMonitoringDataTest;

  public:
    explicit FloodMonitoringData(IHttpClient* client = &defaultClient);

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
    IHttpClient* httpClient;
    static HttpClientAdapter defaultClient;
};
