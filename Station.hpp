#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Station {
public:
    std::string RLOIid;
    std::string catchmentName;
    std::string dateOpened;
    double lat;
    double lon;
    long northing;
    long easting;
    std::string notation;
    std::string town;

    static Station fromJson(const json& jsonObj);
};