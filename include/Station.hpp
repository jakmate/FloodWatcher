#pragma once
#include "Measure.hpp"
#include <simdjson.h>
#include <string>
#include <vector>

class Station {
  public:
    static Station fromJson(const simdjson::dom::element& jsonObj);

    const std::string& getRLOIid() const {
      return RLOIid;
    }
    const std::string& getCatchmentName() const {
      return catchmentName;
    }
    const std::string& getDateOpened() const {
      return dateOpened;
    }
    const std::string& getLabel() const {
      return label;
    }
    double getLat() const {
      return lat;
    }
    double getLon() const {
      return lon;
    }
    long getNorthing() const {
      return northing;
    }
    long getEasting() const {
      return easting;
    }
    const std::string& getNotation() const {
      return notation;
    }
    const std::string& getTown() const {
      return town;
    }
    const std::string& getRiverName() const {
      return riverName;
    }
    const std::string& getStatus() const {
      return status;
    }
    const std::vector<Measure>& getMeasures() const {
      return measures;
    }

    void setMeasures(const std::vector<Measure>& m) {
      measures = m;
    }

  private:
    std::string RLOIid;
    std::string catchmentName;
    std::string dateOpened;
    std::string label;
    double lat = 0.0;
    double lon = 0.0;
    int64_t northing = 0;
    int64_t easting = 0;
    std::string notation;
    std::string town;
    std::string riverName;
    std::string status;
    std::vector<Measure> measures;
};
