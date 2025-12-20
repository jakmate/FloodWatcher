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
  ThreadPool pool(10);

  for (auto& warning : warnings) {
    if (!warning.getPolygonUrl().empty()) {
      std::string url = warning.getPolygonUrl();
      Warning* warningPtr = &warning;
      pool.enqueue([this, url, warningPtr]() {
        auto polygonData = HttpClient::getInstance().fetchUrl(url);
        if (!polygonData) {
          std::cerr << "Failed to fetch polygon from URL: " << url << '\n';
          return;
        }

        try {
          simdjson::dom::parser parser;
          simdjson::dom::element polygonJson;
          auto error = parser.parse(*polygonData).get(polygonJson);
          if (error != 0U) {
            std::cerr << "Error parsing polygon from URL " << url << ": " << error << '\n';
            return;
          }

          // Extract geometry element properly
          simdjson::dom::element geometry;
          error = polygonJson["features"].at(0)["geometry"].get(geometry);
          if (error != 0U) {
            std::cerr << "Error getting geometry from polygon\n";
            return;
          }

          warningPtr->setFloodAreaPolygon(Warning::parseGeoJsonPolygon(geometry));
        } catch (const std::exception& e) {
          std::cerr << "Error parsing polygon from URL " << url << ": " << e.what() << '\n';
        }
      });
    }
  }
}
