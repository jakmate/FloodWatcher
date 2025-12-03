#include "Station.hpp"
#include "HttpClient.hpp"
#include <curl/curl.h>
#include <iostream>

Station Station::fromJson(const json& jsonObj) {
  Station station;

  // Extract value based on status
  auto getActiveValue = [&jsonObj](std::string_view key, const auto& defaultValue) {
    using ValueType = std::remove_cv_t<std::remove_reference_t<decltype(defaultValue)>>;

    if (!jsonObj.contains(key)) {
      return ValueType(defaultValue);
    }

    const auto& field = jsonObj[key];

    // If it's not an array, return as-is
    if (!field.is_array()) {
      return field.get<ValueType>();
    }

    // If an array, find the active index
    const auto& statuses = jsonObj.value("status", json::array());
    if (!statuses.is_array() || statuses.empty()) {
      return field[0].get<ValueType>();
    }

    for (size_t i = 0; i < statuses.size(); ++i) {
      const auto& status = statuses[i].get_ref<const std::string&>();
      if (status.find("statusActive") != std::string::npos) {
        return field[i].get<ValueType>();
      }
    }

    // Fallback to first element
    return field[0].get<ValueType>();
  };

  station.RLOIid = getActiveValue("RLOIid", std::string("unknown"));
  station.catchmentName = getActiveValue("catchmentName", std::string("unknown"));
  station.dateOpened = getActiveValue("dateOpened", std::string("unknown"));
  station.label = getActiveValue("label", std::string("unknown"));
  station.lat = getActiveValue("lat", 0.0);
  station.lon = getActiveValue("long", 0.0);
  station.northing = getActiveValue("northing", 0L);
  station.easting = getActiveValue("easting", 0L);
  station.notation = jsonObj.value("notation", "unknown");
  station.town = jsonObj.value("town", "unknown");
  station.riverName = jsonObj.value("riverName", "unknown");

  return station;
}
