#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Measure
{
public:
    static Measure fromJson(const json &jsonObj);

    const std::string &getId() const { return id; }
    const std::string &getParameter() const { return parameter; }
    const std::string &getParameterName() const { return parameterName; }
    double getPeriod() const { return period; }
    const std::string &getQualifier() const { return qualifier; }
    const std::string &getUnitName() const { return unitName; }

private:
    std::string id;
    std::string parameter;
    std::string parameterName;
    double period;
    std::string qualifier;
    std::string unitName;
};
