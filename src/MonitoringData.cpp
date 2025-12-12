#include "MonitoringData.hpp"
#include <HttpClient.hpp>
#include <future>
#include <iostream>
#include <vector>

void MonitoringData::parseWarnings(const json& apiResponse) {
  if (apiResponse.contains("items") && apiResponse["items"].is_array()) {
    for (const auto& item : apiResponse["items"]) {
      try {
        warnings.push_back(Warning::fromJson(item));
      } catch (const std::exception& e) {
        std::cerr << "Error parsing warning: " << e.what() << '\n';
      }
    }
  }
}

void MonitoringData::parseStations(const json& apiResponse) {
  if (apiResponse.contains("items") && apiResponse["items"].is_array()) {
    for (const auto& item : apiResponse["items"]) {
      try {
        stations.push_back(Station::fromJson(item));
      } catch (const std::exception& e) {
        std::cerr << "Error parsing station: " << e.what() << '\n';
      }
    }
  }
}

void MonitoringData::fetchAllPolygonsAsync() {
  std::vector<std::future<void>> futures;
  std::mutex coutMutex; // For thread-safe logging

  for (auto& warning : warnings) {
    if (!warning.getPolygonUrl().empty()) {
      futures.push_back(std::async(std::launch::async, [this, &warning, &coutMutex]() {
        auto polygonData = HttpClient::getInstance().fetchUrl(warning.getPolygonUrl());
        if (polygonData) {
          try {
            json polygonJson = json::parse(*polygonData);
            warning.setFloodAreaPolygon(
                Warning::parseGeoJsonPolygon(polygonJson["features"][0]["geometry"]));
          } catch (const std::exception& e) {
            std::scoped_lock<std::mutex> lock(coutMutex);
            std::cerr << "Error parsing polygon from URL " << warning.getPolygonUrl() << ": "
                      << e.what() << '\n';
          }
        } else {
          std::scoped_lock<std::mutex> lock(coutMutex);
          std::cerr << "Failed to fetch polygon from URL: " << warning.getPolygonUrl() << '\n';
        }
      }));
    }
  }

  // Wait for all async operations to complete
  for (auto& future : futures) {
    future.get();
  }
}
