#include "Warning.hpp"
#include "HttpClient.hpp"
#include <cmath>
#include <curl/curl.h>
#include <iostream>

Warning Warning::fromJson(const simdjson::dom::element& jsonObj) {
  Warning warning;

  auto getString = [](const simdjson::dom::element& obj, std::string_view key,
                      const char* defaultVal) -> std::string {
    std::string_view sv;
    if (!obj[key].get(sv)) {
      return std::string(sv);
    }
    return defaultVal;
  };

  auto getInt = [](const simdjson::dom::element& obj, std::string_view key, int defaultVal) -> int {
    int64_t val = 0;
    if (!obj[key].get(val)) {
      return static_cast<int>(val);
    }
    return defaultVal;
  };

  warning.id = getString(jsonObj, "floodAreaID", "unknown");
  warning.description = getString(jsonObj, "description", "unknown");
  warning.areaName = getString(jsonObj, "eaAreaName", "unknown");
  warning.severity = getString(jsonObj, "severity", "unknown");
  warning.severityLevel = getInt(jsonObj, "severityLevel", 0);
  warning.timeMessageChanged = getString(jsonObj, "timeMessageChanged", "");
  warning.timeRaised = getString(jsonObj, "timeRaised", "");
  warning.timeSeverityChanged = getString(jsonObj, "timeSeverityChanged", "");
  warning.message = getString(jsonObj, "message", "");

  simdjson::dom::element floodArea;
  if (jsonObj["floodArea"].get(floodArea) == 0U) {
    warning.county = getString(floodArea, "county", "unknown");
    warning.polygonUrl = getString(floodArea, "polygon", "");
  }

  return warning;
}

LinearRing Warning::parseLinearRing(const simdjson::dom::array& ringJson) {
  LinearRing ring;
  ring.reserve(ringJson.size());
  for (auto coord : ringJson) {
    simdjson::dom::array coordArr;
    if (coord.get(coordArr) == 0U && coordArr.size() >= 2) {
      double lon = NAN;
      double lat = NAN;
      if (coordArr.at(0).get(lon) == 0U && coordArr.at(1).get(lat) == 0U) {
        ring.emplace_back(lon, lat);
      }
    }
  }
  return ring;
}

MyPolygon Warning::parsePolygon(const simdjson::dom::array& polygonJson) {
  MyPolygon polygon;
  polygon.reserve(polygonJson.size());
  for (auto ringJson : polygonJson) {
    simdjson::dom::array ringArr;
    if (ringJson.get(ringArr) == 0U) {
      LinearRing ring = parseLinearRing(ringArr);
      if (!ring.empty()) {
        polygon.push_back(std::move(ring));
      }
    }
  }
  return polygon;
}

MultiPolygon Warning::parseGeoJsonPolygon(const simdjson::dom::element& geoJson) {
  MultiPolygon result;

  simdjson::dom::array coords;
  if (geoJson["coordinates"].get(coords) != 0U) {
    return result;
  }

  std::string_view typeView;
  std::string type;
  if (geoJson["type"].get(typeView) == 0U) {
    type = std::string(typeView);
  }

  if (type == "Polygon") {
    MyPolygon polygon = parsePolygon(coords);
    if (!polygon.empty()) {
      result.push_back(std::move(polygon));
    }
  } else if (type == "MultiPolygon") {
    result.reserve(coords.size());
    for (auto polygonCoords : coords) {
      simdjson::dom::array polygonArr;
      if (polygonCoords.get(polygonArr) == 0U) {
        MyPolygon polygon = parsePolygon(polygonArr);
        if (!polygon.empty()) {
          result.push_back(std::move(polygon));
        }
      }
    }
  } else {
    std::cerr << "Error parsing polygon - Unknown structure";
  }

  return result;
}
