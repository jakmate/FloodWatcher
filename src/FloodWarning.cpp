#include "FloodWarning.hpp"
#include "HttpClient.hpp"
#include <curl/curl.h>
#include <iostream>

FloodWarning FloodWarning::fromJson(const json& jsonObj) {
  FloodWarning warning;
  warning.id = jsonObj.value("floodAreaID", "unknown");
  warning.description = jsonObj.value("description", "unknown");
  warning.areaName = jsonObj.value("eaAreaName", "unknown");
  warning.severity = jsonObj.value("severity", "unknown");
  warning.severityLevel = jsonObj.value("severityLevel", 0);
  warning.timeMessageChanged = jsonObj.value("timeMessageChanged", "");
  warning.timeRaised = jsonObj.value("timeRaised", "");
  warning.timeSeverityChanged = jsonObj.value("timeSeverityChanged", "");

  if (jsonObj.contains("floodArea") && jsonObj["floodArea"].is_object()) {
    const auto& floodArea = jsonObj["floodArea"];
    warning.region = floodArea.value("region", "unknown");
    warning.county = floodArea.value("county", "unknown");

    if (floodArea.contains("polygon")) {
      try {
        std::string polygonUrl = floodArea.value("polygon", "");
        // std::cout << "Fetching polygon from: " << polygonUrl;

        auto polygonData = fetchUrl(polygonUrl);
        if (polygonData) {
          try {
            json polygonJson = json::parse(*polygonData);
            warning.floodAreaPolygon = parseGeoJsonPolygon(polygonJson["features"][0]["geometry"]);
          } catch (const std::exception& e) {
            std::cerr << "Error parsing polygon from URL: " << e.what();
          }
        }
      } catch (const std::exception& e) {
        std::cerr << "Error parsing embedded polygon: " << e.what();
      }
    }
  }

  return warning;
}

LinearRing FloodWarning::parseLinearRing(const json& ringJson) {
  LinearRing ring;
  for (const auto& coord : ringJson) {
    if (coord.is_array() && coord.size() >= 2) {
      ring.emplace_back(coord[0].get<double>(), coord[1].get<double>());
    }
  }
  return ring;
}

MyPolygon FloodWarning::parsePolygon(const json& polygonJson) {
  MyPolygon polygon;
  for (const auto& ringJson : polygonJson) {
    LinearRing ring = parseLinearRing(ringJson);
    if (!ring.empty()) {
      polygon.push_back(ring);
    }
  }
  return polygon;
}

MultiPolygon FloodWarning::parseGeoJsonPolygon(const json& geoJson) {
  MultiPolygon result;
  auto type = geoJson.value("type", "");

  if (!geoJson.contains("coordinates") || !geoJson["coordinates"].is_array()) {
    return result;
  }

  if (type == "Polygon") {
    MyPolygon polygon = parsePolygon(geoJson["coordinates"]);
    if (!polygon.empty()) {
      result.push_back(polygon);
    }
  } else if (type == "MultiPolygon") {
    for (const auto& polygonCoords : geoJson["coordinates"]) {
      MyPolygon polygon = parsePolygon(polygonCoords);
      if (!polygon.empty()) {
        result.push_back(polygon);
      }
    }
  } else {
    std::cerr << "Error parsing polygon - Unknown structure";
  }

  return result;
}
