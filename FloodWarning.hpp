#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "GeometryTypes.hpp"

using json = nlohmann::json;

class FloodWarning {
public:
    static FloodWarning fromJson(const json& jsonObj);

    const std::string& getId() const { return id; }
    const std::string& getDescription() const { return description; }
    const std::string& getAreaName() const { return areaName; }
    const std::string& getSeverity() const { return severity; }
    int getSeverityLevel() const { return severityLevel; }
    const std::string& getTimeMessageChanged() const { return timeMessageChanged; }
    const std::string& getTimeRaised() const { return timeRaised; }
    const std::string& getTimeSeverityChanged() const { return timeSeverityChanged; }
    const std::string& getRegion() const { return region; }
    const std::string& getCounty() const { return county; }
    const std::optional<MultiPolygon>& getFloodAreaPolygon() const { return floodAreaPolygon; }

private:
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

    static MultiPolygon parseGeoJsonPolygon(const json& geoJson);
};