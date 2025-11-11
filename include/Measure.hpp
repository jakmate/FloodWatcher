#pragma once
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Measure {
  public:
    static Measure fromJson(const json& jsonObj);

    const std::string& getId() const {
      return id;
    }
    const std::string& getParameter() const {
      return parameter;
    }
    const std::string& getParameterName() const {
      return parameterName;
    }
    double getPeriod() const {
      return period;
    }
    const std::string& getQualifier() const {
      return qualifier;
    }
    const std::string& getUnitName() const {
      return unitName;
    }
    double getLatestReading() const {
      return latestReading;
    }

  private:
    std::string id;
    std::string parameter;
    std::string parameterName;
    double period = 0.0;
    std::string qualifier;
    std::string unitName;
    double latestReading = 0.0;
};
