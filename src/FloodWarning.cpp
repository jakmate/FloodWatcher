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
  warning.message = jsonObj.value("message", "");

  if (jsonObj.contains("floodArea") && jsonObj["floodArea"].is_object()) {
    const auto& floodArea = jsonObj["floodArea"];
    warning.county = floodArea.value("county", "unknown");
    warning.polygonUrl = floodArea.value("polygon", "");
  }

  return warning;
}

LinearRing FloodWarning::parseLinearRing(const json& ringJson) {
  LinearRing ring;
  ring.reserve(ringJson.size());
  for (const auto& coord : ringJson) {
    if (coord.is_array() && coord.size() >= 2) {
      ring.emplace_back(coord[0].get<double>(), coord[1].get<double>());
    }
  }
  return ring;
}

MyPolygon FloodWarning::parsePolygon(const json& polygonJson) {
  MyPolygon polygon;
  polygon.reserve(polygonJson.size());
  for (const auto& ringJson : polygonJson) {
    LinearRing ring = parseLinearRing(ringJson);
    if (!ring.empty()) {
      polygon.push_back(std::move(ring));
    }
  }
  return polygon;
}

MultiPolygon FloodWarning::parseGeoJsonPolygon(const json& geoJson) {
  MultiPolygon result;

  if (!geoJson.contains("coordinates") || !geoJson["coordinates"].is_array()) {
    return result;
  }

  const auto& coords = geoJson["coordinates"];
  auto type = geoJson.value("type", "");

  if (type == "Polygon") {
    MyPolygon polygon = parsePolygon(coords);
    if (!polygon.empty()) {
      result.push_back(std::move(polygon));
    }
  } else if (type == "MultiPolygon") {
    result.reserve(coords.size());
    for (const auto& polygonCoords : coords) {
      MyPolygon polygon = parsePolygon(polygonCoords);
      if (!polygon.empty()) {
        result.push_back(std::move(polygon));
      }
    }
  } else {
    std::cerr << "Error parsing polygon - Unknown structure";
  }

  return result;
}
