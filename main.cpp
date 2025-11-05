#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "FloodWarning.hpp"
#include "Station.hpp"
#include "GeometryTypes.hpp"
#include "HttpClient.hpp"

using json = nlohmann::json;

class FloodMonitoringData {
public:
    std::vector<FloodWarning> floodWarnings;
    std::vector<Station> stations;

    // Parse entire API response
    void parseFloodWarnings(const json& apiResponse) {
        if (apiResponse.contains("items") && apiResponse["items"].is_array()) {
            for (const auto& item : apiResponse["items"]) {
                floodWarnings.push_back(FloodWarning::fromJson(item));
            }
        }
    }

    void parseStations(const json& apiResponse) {
        if (apiResponse.contains("items") && apiResponse["items"].is_array()) {
            for (const auto& item : apiResponse["items"]) {
                try {
                    stations.push_back(Station::fromJson(item));
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing station: " << e.what() << std::endl;
                    continue;
                }
            }
        } else {
            std::cerr << "No items array found in API response" << std::endl;
        }
    }
};


int main() {
    auto response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");
    
    if (!response) {
        std::cerr << "Failed to fetch flood warning data" << std::endl;
        return 1;
    }
    
    std::string readBuffer = *response;

    FloodMonitoringData monitoringData;

    try {
        json data = json::parse(readBuffer);
        monitoringData.parseFloodWarnings(data);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        std::cerr << "Raw response:\n" << readBuffer << std::endl;
        return 1;
    }

    response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/stations");
    
    if (!response) {
        std::cerr << "Failed to fetch stations data" << std::endl;
        return 1;
    }
    
    readBuffer = *response;

    try {
        json data = json::parse(readBuffer);
        monitoringData.parseStations(data);

        std::cout << "Found " << monitoringData.stations.size() << " stations\n";

        for (const auto& station : monitoringData.stations) {
            std::cout << "\n" << station.RLOIid << " in " << station.town << "\n";
            std::cout << station.lat << ", " << station.lon << "\n";
            std::cout << station.northing << ", " << station.easting << "\n";
            std::cout << "-----------------" << "\n";
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        std::cerr << "Raw response:\n" << readBuffer << std::endl;
        return 1;
    }

    return 0;
}