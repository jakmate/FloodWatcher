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
      try {
        stations.push_back(Station::fromJson(item));
      } catch (const std::exception& e) {
        std::cerr << "Error parsing station: " << e.what() << '\n';
      }
    }
  }
}

void MonitoringData::fetchAllPolygonsAsync() {
  ThreadPool pool(10);

  for (auto& warning : warnings) {
    if (!warning.getPolygonUrl().empty()) {
      pool.enqueue([this, &warning]() {
        auto polygonData = HttpClient::getInstance().fetchUrl(warning.getPolygonUrl());
        if (polygonData) {
          try {
            simdjson::dom::parser parser;
            simdjson::dom::element polygonJson;
            auto error = parser.parse(*polygonData).get(polygonJson);
            if (error != 0U) {
              std::cerr << "Error parsing polygon from URL " << warning.getPolygonUrl() << ": "
                        << error << '\n';
              return;
            }

            // Extract geometry element properly
            simdjson::dom::element features;
            error = polygonJson["features"].get(features);
            if (error != 0U) {
              std::cerr << "Error getting features from polygon\n";
              return;
            }

            simdjson::dom::element firstFeature;
            error = features.at(0).get(firstFeature);
            if (error != 0U) {
              std::cerr << "Error getting first feature\n";
              return;
            }

            simdjson::dom::element geometry;
            error = firstFeature["geometry"].get(geometry);
            if (error != 0U) {
              std::cerr << "Error getting geometry\n";
              return;
            }

            warning.setFloodAreaPolygon(Warning::parseGeoJsonPolygon(geometry));
          } catch (const std::exception& e) {
            std::cerr << "Error parsing polygon from URL " << warning.getPolygonUrl() << ": "
                      << e.what() << '\n';
          }
        } else {
          std::cerr << "Failed to fetch polygon from URL: " << warning.getPolygonUrl() << '\n';
        }
      });
    }
  }
}
