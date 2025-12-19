#include "Station.hpp"
#include "HttpClient.hpp"
#include <cmath>
#include <curl/curl.h>
#include <iostream>

Station Station::fromJson(const simdjson::dom::element& jsonObj) {
  Station station;

  // Extract value based on status
  auto getActiveValue = [&jsonObj](std::string_view key, const auto& defaultValue) {
    using ValueType = std::remove_cv_t<std::remove_reference_t<decltype(defaultValue)>>;

    simdjson::dom::element field;
    auto error = jsonObj[key].get(field);
    if (error != 0U) {
      return ValueType(defaultValue);
    }

    // If it's not an array, return as-is
    simdjson::dom::array arr;
    if (field.get(arr)) {
      // Not an array, get the value directly
      if constexpr (std::is_same_v<ValueType, std::string>) {
        std::string_view sv;
        if (!field.get(sv)) {
          return std::string(sv);
        }
      } else if constexpr (std::is_same_v<ValueType, double>) {
        double val = NAN;
        if (!field.get(val)) {
          return val;
        }
      } else if constexpr (std::is_same_v<ValueType, int64_t>) {
        int64_t val = 0;
        if (!field.get(val)) {
          return val;
        }
      }
      return ValueType(defaultValue);
    }

    // If an array, find the active index
    simdjson::dom::array statuses;
    error = jsonObj["status"].get(statuses);
    if (error || statuses.size() == 0) {
      // Get first element
      if constexpr (std::is_same_v<ValueType, std::string>) {
        std::string_view sv;
        if (!arr.at(0).get(sv)) {
          return std::string(sv);
        }
      } else if constexpr (std::is_same_v<ValueType, double>) {
        double val = NAN;
        if (!arr.at(0).get(val)) {
          return val;
        }
      } else if constexpr (std::is_same_v<ValueType, int64_t>) {
        int64_t val = 0;
        if (!arr.at(0).get(val)) {
          return val;
        }
      }
      return ValueType(defaultValue);
    }

    size_t i = 0;
    for (auto status : statuses) {
      std::string_view statusStr;
      if (!status.get(statusStr)) {
        if (statusStr.find("statusActive") != std::string_view::npos) {
          // Get value at index i
          if constexpr (std::is_same_v<ValueType, std::string>) {
            std::string_view sv;
            if (!arr.at(i).get(sv)) {
              return std::string(sv);
            }
          } else if constexpr (std::is_same_v<ValueType, double>) {
            double val = NAN;
            if (!arr.at(i).get(val)) {
              return val;
            }
          } else if constexpr (std::is_same_v<ValueType, int64_t>) {
            int64_t val = 0;
            if (!arr.at(i).get(val)) {
              return val;
            }
          }
        }
      }
      i++;
    }

    // Fallback to first element
    if constexpr (std::is_same_v<ValueType, std::string>) {
      std::string_view sv;
      if (!arr.at(0).get(sv)) {
        return std::string(sv);
      }
    } else if constexpr (std::is_same_v<ValueType, double>) {
      double val = NAN;
      if (!arr.at(0).get(val)) {
        return val;
      }
    } else if constexpr (std::is_same_v<ValueType, int64_t>) {
      int64_t val = 0;
      if (!arr.at(0).get(val)) {
        return val;
      }
    }
    return ValueType(defaultValue);
  };

  station.RLOIid = getActiveValue("RLOIid", std::string("unknown"));
  station.catchmentName = getActiveValue("catchmentName", std::string("unknown"));
  station.dateOpened = getActiveValue("dateOpened", std::string("unknown"));
  station.label = getActiveValue("label", std::string("unknown"));
  station.lat = getActiveValue("lat", 0.0);
  station.lon = getActiveValue("long", 0.0);
  station.northing = getActiveValue("northing", 0L);
  station.easting = getActiveValue("easting", 0L);

  std::string_view notationView;
  if (jsonObj["notation"].get(notationView) == 0U) {
    station.notation = std::string(notationView);
  } else {
    station.notation = "unknown";
  }

  std::string_view townView;
  if (jsonObj["town"].get(townView) == 0U) {
    station.town = std::string(townView);
  } else {
    station.town = "unknown";
  }

  std::string_view riverView;
  if (jsonObj["riverName"].get(riverView) == 0U) {
    station.riverName = std::string(riverView);
  } else {
    station.riverName = "unknown";
  }

  return station;
}
