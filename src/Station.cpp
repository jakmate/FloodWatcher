#include "Station.hpp"
#include "HttpClient.hpp"
#include <TypeUtils.hpp>
#include <curl/curl.h>
#include <iostream>

Station Station::fromJson(const simdjson::dom::element& jsonObj) {
  Station station;

  station.RLOIid = getActiveValue(jsonObj, "RLOIid", std::string("unknown"));
  station.catchmentName = getActiveValue(jsonObj, "catchmentName", std::string("unknown"));
  station.dateOpened = getActiveValue(jsonObj, "dateOpened", std::string("unknown"));
  station.label = getActiveValue(jsonObj, "label", std::string("unknown"));
  station.lat = getActiveValue(jsonObj, "lat", 0.0);
  station.lon = getActiveValue(jsonObj, "long", 0.0);
  station.northing = getActiveValue(jsonObj, "northing", int64_t(0));
  station.easting = getActiveValue(jsonObj, "easting", int64_t(0));
  station.notation = getString(jsonObj, "notation", "unknown");
  station.town = getString(jsonObj, "town", "unknown");
  station.riverName = getString(jsonObj, "riverName", "unknown");

  return station;
}
