#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "Measure.hpp"

using json = nlohmann::json;

class Station
{
public:
    static Station fromJson(const json &jsonObj);

    const std::string &getRLOIid() const { return RLOIid; }
    const std::string &getCatchmentName() const { return catchmentName; }
    const std::string &getDateOpened() const { return dateOpened; }
    const std::string &getLabel() const { return label; }
    double getLat() const { return lat; }
    double getLon() const { return lon; }
    int getNorthing() const { return northing; }
    int getEasting() const { return easting; }
    const std::string &getNotation() const { return notation; }
    const std::string &getTown() const { return town; }
    const std::string &getRiverName() const { return riverName; }
    const std::string &getStatus() const { return status; }
    const std::vector<Measure> &getMeasures() const { return measures; }

private:
    std::string RLOIid;
    std::string catchmentName;
    std::string dateOpened;
    std::string label;
    double lat;
    double lon;
    int northing;
    int easting;
    std::string notation;
    std::string town;
    std::string riverName;
    std::string status;
    std::vector<Measure> measures;
};