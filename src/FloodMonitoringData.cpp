#include "FloodMonitoringData.hpp"
#include <HttpClient.hpp>
#include <future>
#include <iostream>
#include <vector>

void FloodMonitoringData::parseFloodWarnings(const json& apiResponse) {
  if (apiResponse.contains("items") && apiResponse["items"].is_array()) {
    for (const auto& item : apiResponse["items"]) {
      try {
        floodWarnings.push_back(FloodWarning::fromJson(item));
      } catch (const std::exception& e) {
        std::cerr << "Error parsing flood warning: " << e.what() << '\n';
      }
    }
  }
}

void FloodMonitoringData::parseStations(const json& apiResponse) {
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

void FloodMonitoringData::fetchAllPolygonsAsync() {
  std::vector<std::future<void>> futures;
  std::mutex coutMutex; // For thread-safe logging

  for (auto& warning : floodWarnings) {
    if (!warning.getPolygonUrl().empty()) {
      futures.push_back(std::async(std::launch::async, [&warning, &coutMutex]() {
        auto polygonData = HttpClient::fetchUrl(warning.getPolygonUrl());
        if (polygonData) {
          try {
            json polygonJson = json::parse(*polygonData);
            warning.setFloodAreaPolygon(
                FloodWarning::parseGeoJsonPolygon(polygonJson["features"][0]["geometry"]));
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
