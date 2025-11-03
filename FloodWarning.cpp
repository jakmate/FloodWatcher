#include <iostream>
#include <curl/curl.h>

#include "FloodWarning.hpp"
#include "HttpClient.hpp"

FloodWarning FloodWarning::fromJson(const json& jsonObj) {
    FloodWarning warning;
    warning.id = jsonObj["floodAreaID"].get<std::string>();
    warning.description = jsonObj["description"].get<std::string>();
    warning.areaName = jsonObj["eaAreaName"].get<std::string>();
    warning.severity = jsonObj["severity"].get<std::string>();
    warning.severityLevel = jsonObj["severityLevel"].get<int>();
    
    if (jsonObj.contains("timeMessageChanged") && !jsonObj["timeMessageChanged"].is_null()) {
        warning.timeMessageChanged = jsonObj["timeMessageChanged"].get<std::string>();
    }
    
    if (jsonObj.contains("timeRaised") && !jsonObj["timeRaised"].is_null()) {
        warning.timeRaised = jsonObj["timeRaised"].get<std::string>();
    }

    if (jsonObj.contains("timeSeverityChanged") && !jsonObj["timeSeverityChanged"].is_null()) {
        warning.timeSeverityChanged = jsonObj["timeSeverityChanged"].get<std::string>();
    }
    
    if (jsonObj.contains("floodArea") && jsonObj["floodArea"].is_object()) {
        auto& floodArea = jsonObj["floodArea"];
        if (floodArea.contains("region") && !floodArea["region"].is_null()) {
            warning.region = floodArea["region"].get<std::string>();
        }
        if (floodArea.contains("county") && !floodArea["county"].is_null()) {
            warning.county = floodArea["county"].get<std::string>();
        }
        
        // Check if polygon is a URL string (not embedded GeoJSON)
        if (floodArea.contains("polygon") && !floodArea["polygon"].is_null()) {
            if (floodArea["polygon"].is_string()) {
                // Polygon is a URL - fetch it
                std::string polygonUrl = floodArea["polygon"].get<std::string>();
                std::cout << "Fetching polygon from: " << polygonUrl << std::endl;
                
                auto polygonData = fetchUrl(polygonUrl);
                if (polygonData) {
                    try {
                        json polygonJson = json::parse(*polygonData);
                        warning.floodAreaPolygon = parseGeoJsonPolygon(polygonJson["features"][0]["geometry"]);
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing polygon from URL: " << e.what() << std::endl;
                    }
                }
            } else {
                // Polygon is embedded GeoJSON
                try {
                    warning.floodAreaPolygon = parseGeoJsonPolygon(floodArea["polygon"]);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing embedded polygon: " << e.what() << std::endl;
                }
            }
        }
    }
    
    return warning;
}

MultiPolygon FloodWarning::parseGeoJsonPolygon(const json& geoJson) {
    MultiPolygon result;
    
    if (geoJson.contains("type") && geoJson["type"] == "Polygon") {
        // Single polygon: { "type": "Polygon", "coordinates": [[[lon,lat],...]] }
        if (geoJson.contains("coordinates") && geoJson["coordinates"].is_array()) {
            MyPolygon polygon;
            for (const auto& ring : geoJson["coordinates"]) {
                LinearRing linearRing;
                for (const auto& coord : ring) {
                    if (coord.is_array() && coord.size() >= 2) {
                        double lon = coord[0].get<double>();
                        double lat = coord[1].get<double>();
                        linearRing.emplace_back(lon, lat);
                    }
                }
                if (!linearRing.empty()) {
                    polygon.push_back(linearRing);
                }
            }
            if (!polygon.empty()) {
                result.push_back(polygon);
            }
        }
    }
    else if (geoJson.contains("type") && geoJson["type"] == "MultiPolygon") {
        // MultiPolygon: { "type": "MultiPolygon", "coordinates": [[[[lon,lat],...]]] }
        if (geoJson.contains("coordinates") && geoJson["coordinates"].is_array()) {
            for (const auto& polygonCoords : geoJson["coordinates"]) {
                MyPolygon polygon;
                for (const auto& ring : polygonCoords) {
                    LinearRing linearRing;
                    for (const auto& coord : ring) {
                        if (coord.is_array() && coord.size() >= 2) {
                            double lon = coord[0].get<double>();
                            double lat = coord[1].get<double>();
                            linearRing.emplace_back(lon, lat);
                        }
                    }
                    if (!linearRing.empty()) {
                        polygon.push_back(linearRing);
                    }
                }
                if (!polygon.empty()) {
                    result.push_back(polygon);
                }
            }
        }
    }
    else if (geoJson.is_array()) {
        // Sometimes the API returns raw coordinates without GeoJSON wrapper
        for (const auto& polygonCoords : geoJson) {
            if (polygonCoords.is_array()) {
                MyPolygon polygon;
                for (const auto& ring : polygonCoords) {
                    LinearRing linearRing;
                    if (ring.is_array()) {
                        for (const auto& coord : ring) {
                            if (coord.is_array() && coord.size() >= 2) {
                                double lon = coord[0].get<double>();
                                double lat = coord[1].get<double>();
                                linearRing.emplace_back(lon, lat);
                            }
                        }
                    }
                    if (!linearRing.empty()) {
                        polygon.push_back(linearRing);
                    }
                }
                if (!polygon.empty()) {
                    result.push_back(polygon);
                }
            }
        }
    }
    
    return result;
};