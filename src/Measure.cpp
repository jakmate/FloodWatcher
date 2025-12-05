#include "Measure.hpp"
#include <iostream>

Measure Measure::fromJson(const json& jsonObj) {
  Measure measure;
  measure.id = jsonObj.value("@id", "");
  measure.parameter = jsonObj.value("parameter", "");
  measure.parameterName = jsonObj.value("parameterName", "");
  measure.period = jsonObj.value("period", 0.0);
  measure.qualifier = jsonObj.value("qualifier", "");
  measure.unitName = jsonObj.value("unitName", "");

  if (jsonObj.contains("latestReading")) {
    const auto& reading = jsonObj["latestReading"];
    if (reading.is_object() && reading.contains("value")) {
      measure.latestReading = reading["value"].get<double>();
    }
  }

  return measure;
}
