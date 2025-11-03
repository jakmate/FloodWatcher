#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

#include "FloodWarning.hpp"
#include "GeometryTypes.hpp"
#include "HttpClient.hpp"

using json = nlohmann::json;

class FloodMonitoringData {
public:
    std::vector<FloodWarning> floodWarnings;

    // Parse entire API response
    void parseFloodWarnings(const json& apiResponse) {
        if (apiResponse.contains("items") && apiResponse["items"].is_array()) {
            for (const auto& item : apiResponse["items"]) {
                floodWarnings.push_back(FloodWarning::fromJson(item));
            }
        }
    }
};


int main() {
    auto response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");
    
    if (!response) {
        std::cerr << "Failed to fetch flood data" << std::endl;
        return 1;
    }
    
    std::string readBuffer = *response;

    FloodMonitoringData monitoringData;

    try {
        json data = json::parse(readBuffer);
        monitoringData.parseFloodWarnings(data);

        for (const auto& warning : monitoringData.floodWarnings) {
            std::cout << "\n" << warning.severity << " in " << warning.areaName << "\n";
            std::cout << warning.description << "\n";
            if (warning.floodAreaPolygon) {
                std::cout << "FLOOD AREA POLYGON DATA:\n";
                printMultiPolygon(*warning.floodAreaPolygon, "  ");
            } else {
                std::cout << "No polygon data available for this warning\n";
            }
            std::cout << "-----------------" << "\n";
        } 
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        std::cerr << "Raw response:\n" << readBuffer << std::endl;
        return 1;
    }

    return 0;
}