#include "MonitoringData.hpp"
#include <HttpClient.hpp>
#include <ThreadPool.hpp>
#include <future>
#include <iostream>
#include <vector>

void MonitoringData::parseWarnings(const simdjson::dom::element& apiResponse) {
  simdjson::dom::array items;
  auto error = apiResponse["items"].get(items);
  if (error == 0U) {
    for (auto item : items) {
      if (item.is_null()) {
        continue;
      }

      try {
        warnings.push_back(Warning::fromJson(item));
      } catch (const std::exception& e) {
        std::cerr << "Error parsing warning: " << e.what() << '\n';
      }
    }
  }
}

void MonitoringData::parseStations(const simdjson::dom::element& apiResponse) {
  simdjson::dom::array items;
  auto error = apiResponse["items"].get(items);
  if (error == 0U) {
    for (auto item : items) {
      if (item.is_null()) {
        continue;
      }

      try {
        stations.push_back(Station::fromJson(item));
      } catch (const std::exception& e) {
        std::cerr << "Error parsing station: " << e.what() << '\n';
      }
    }
  }
}

void MonitoringData::fetchAllPolygonsAsync() {
  // Collect all URLs that need fetching
  std::vector<std::string> urls;
  std::vector<Warning*> warningPtrs;

  for (auto& warning : warnings) {
    if (!warning.getPolygonUrl().empty()) {
      urls.push_back(warning.getPolygonUrl());
      warningPtrs.push_back(&warning);
    }
  }

  if (urls.empty()) {
    return;
  }

  // Fetch all URLs concurrently using multi-curl
  auto results = HttpClient::getInstance().fetchUrls(urls);

  // Process results
  simdjson::dom::parser parser;
  for (size_t i = 0; i < results.size(); ++i) {
    if (!results[i]) {
      std::cerr << "Failed to fetch polygon from URL: " << urls[i] << '\n';
      continue;
    }

    try {
      simdjson::dom::element polygonJson;
      auto error = parser.parse(*results[i]).get(polygonJson);
      if (error != 0U) {
        std::cerr << "Error parsing polygon from URL " << urls[i] << ": " << error << '\n';
        continue;
      }

      // Extract geometry element properly
      simdjson::dom::element geometry;
      error = polygonJson["features"].at(0)["geometry"].get(geometry);
      if (error != 0U) {
        std::cerr << "Error getting geometry from polygon\n";
        continue;
      }

      warningPtrs[i]->setFloodAreaPolygon(Warning::parseGeoJsonPolygon(geometry));
    } catch (const std::exception& e) {
      std::cerr << "Error parsing polygon from URL " << urls[i] << ": " << e.what() << '\n';
    }
  }
}
