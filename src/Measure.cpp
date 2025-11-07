#include <iostream>
#include "Measure.hpp"

Measure Measure::fromJson(const json &jsonObj)
{
    Measure measure;
    measure.id = jsonObj.value("@id", "");
    measure.parameter = jsonObj.value("parameter", "");
    measure.parameterName = jsonObj.value("parameterName", "");
    measure.period = jsonObj.value("period", 0.0);
    measure.qualifier = jsonObj.value("qualifier", "");
    measure.unitName = jsonObj.value("unitName", "");
    return measure;
}