#include <string>
#include <optional>
#include <nlohmann/json.hpp>

#include "GeometryTypes.hpp"

using json = nlohmann::json;

class FloodWarning {
public:
    std::string id;
    std::string description;
    std::string areaName;
    std::string severity;
    int severityLevel;
    std::string timeMessageChanged;
    std::string timeRaised;
    std::string timeSeverityChanged;
    std::string region;
    std::string county;
    std::optional<MultiPolygon> floodAreaPolygon;

    static FloodWarning fromJson(const json& jsonObj);

private:
    static MultiPolygon parseGeoJsonPolygon(const json& geoJson);
};