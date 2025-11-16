#pragma once
#include "GeometryTypes.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

using json = nlohmann::json;

class FloodWarning {
  public:
    static FloodWarning fromJson(const json& jsonObj);
    static MultiPolygon parseGeoJsonPolygon(const json& geoJson);

    const std::string& getId() const {
      return id;
    }
    const std::string& getDescription() const {
      return description;
    }
    const std::string& getAreaName() const {
      return areaName;
    }
    const std::string& getSeverity() const {
      return severity;
    }
    int getSeverityLevel() const {
      return severityLevel;
    }
    const std::string& getTimeMessageChanged() const {
      return timeMessageChanged;
    }
    const std::string& getTimeRaised() const {
      return timeRaised;
    }
    const std::string& getTimeSeverityChanged() const {
      return timeSeverityChanged;
    }
    const std::string& getMessage() const {
      return message;
    }
    const std::string& getRegion() const {
      return region;
    }
    const std::string& getCounty() const {
      return county;
    }
    const std::string& getPolygonUrl() const {
      return polygonUrl;
    }
    const std::optional<MultiPolygon>& getFloodAreaPolygon() const {
      return floodAreaPolygon;
    }

    void setFloodAreaPolygon(const std::optional<MultiPolygon>& polygon) {
      floodAreaPolygon = polygon;
    }

  private:
    std::string id;
    std::string description;
    std::string areaName;
    std::string severity;
    int severityLevel = 0;
    std::string timeMessageChanged;
    std::string timeRaised;
    std::string timeSeverityChanged;
    std::string message;
    std::string region;
    std::string county;
    std::string polygonUrl;
    std::optional<MultiPolygon> floodAreaPolygon;

    static LinearRing parseLinearRing(const json& ringJson);
    static MyPolygon parsePolygon(const json& polygonJson);
};
