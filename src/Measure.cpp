#include "Measure.hpp"
#include <cmath>
#include <iostream>

Measure Measure::fromJson(const simdjson::dom::element& jsonObj) {
  Measure measure;

  auto getString = [](const simdjson::dom::element& obj, std::string_view key,
                      const char* defaultVal) -> std::string {
    std::string_view sv;
    if (!obj[key].get(sv)) {
      return std::string(sv);
    }
    return defaultVal;
  };

  measure.id = getString(jsonObj, "@id", "");
  measure.parameter = getString(jsonObj, "parameter", "");
  measure.parameterName = getString(jsonObj, "parameterName", "");
  measure.qualifier = getString(jsonObj, "qualifier", "");
  measure.unitName = getString(jsonObj, "unitName", "");

  double periodVal = NAN;
  if (jsonObj["period"].get(periodVal) == 0U) {
    measure.period = periodVal;
  } else {
    measure.period = 0.0;
  }

  simdjson::dom::element reading;
  if (jsonObj["latestReading"].get(reading) == 0U) {
    double value = NAN;
    if (reading["value"].get(value) == 0U) {
      measure.latestReading = value;
    }
  }

  return measure;
}
