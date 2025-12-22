#pragma once
#include "GeometryTypes.hpp"
#include <optional>
#include <simdjson.h>
#include <string>

class Warning {
  public:
    static Warning fromJson(const simdjson::dom::element& jsonObj);
    static MultiPolygon parseGeoJsonPolygon(const simdjson::dom::element& geoJson);

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
    std::string county;
    std::string polygonUrl;
    std::optional<MultiPolygon> floodAreaPolygon;

    static LinearRing parseLinearRing(const simdjson::dom::array& ringJson);
    static MyPolygon parsePolygon(const simdjson::dom::array& polygonJson);
};
