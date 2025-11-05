#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include "FloodMonitoringData.hpp"
#include "HttpClient.hpp"

using json = nlohmann::json;

int main() {
    FloodMonitoringData monitoringData;

    auto response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");
    
    if (!response) {
        std::cerr << "Failed to fetch flood warning data" << std::endl;
        return 1;
    }
    
    std::string readBuffer = *response;

    try {
        json data = json::parse(readBuffer);
        monitoringData.parseFloodWarnings(data);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        std::cerr << "Raw response:\n" << readBuffer << std::endl;
        return 1;
    }

    response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/stations?status=Active");
    
    if (!response) {
        std::cerr << "Failed to fetch stations data" << std::endl;
        return 1;
    }
    
    readBuffer = *response;

    try {
        json data = json::parse(readBuffer);
        monitoringData.parseStations(data);

        std::cout << "Found " << monitoringData.getStations().size() << " stations\n";

        for (const auto& station : monitoringData.getStations()) {
            std::cout << "\n" << station.getRLOIid() << " in " << station.getTown() << "\n";
            std::cout << station.getLat() << ", " << station.getLon() << "\n";
            std::cout << station.getNorthing() << ", " << station.getEasting() << "\n";
            for (const auto& measure : station.getMeasures()) {
                std::cout << "Parameter: " << measure.getParameterName() 
                        << " (" << measure.getQualifier() << ")\n"
                        << "Unit: " << measure.getUnitName() << "\n"
                        << "Period: " << measure.getPeriod() << "s\n"
                        << "ID: " << measure.getId() << "\n\n";
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