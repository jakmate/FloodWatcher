#include "Measure.hpp"
#include "TypeUtils.hpp"
#include <iostream>

Measure Measure::fromJson(const simdjson::dom::element& jsonObj) {
  Measure measure;

  measure.id = getString(jsonObj, "@id", "");
  measure.parameter = getString(jsonObj, "parameter", "");
  measure.parameterName = getString(jsonObj, "parameterName", "");
  measure.qualifier = getString(jsonObj, "qualifier", "");
  measure.unitName = getString(jsonObj, "unitName", "");
  measure.period = getDouble(jsonObj, "period", 0.0);

  simdjson::dom::element reading;
  if (jsonObj["latestReading"].get(reading) == 0U) {
    measure.latestReading = getDouble(reading, "value", 0.0);
  }

  return measure;
}
