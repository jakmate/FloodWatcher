#include <iostream>
#include <curl/curl.h>
#include "FloodWarning.hpp"
#include "HttpClient.hpp"

FloodWarning FloodWarning::fromJson(const json &jsonObj)
{
    FloodWarning warning;
    warning.id = jsonObj.value("floodAreaID", "unknown");
    warning.description = jsonObj.value("description", "unknown");
    warning.areaName = jsonObj.value("eaAreaName", "unknown");
    warning.severity = jsonObj.value("severity", "unknown");
    warning.severityLevel = jsonObj.value("severityLevel", 0);
    warning.timeMessageChanged = jsonObj.value("timeMessageChanged", "");
    warning.timeRaised = jsonObj.value("timeRaised", "");
    warning.timeSeverityChanged = jsonObj.value("timeSeverityChanged", "");

    if (jsonObj.contains("floodArea") && jsonObj["floodArea"].is_object())
    {
        auto &floodArea = jsonObj["floodArea"];
        warning.region = floodArea.value("region", "unknown");
        warning.county = floodArea.value("county", "unknown");

        if (floodArea.contains("polygon"))
        {
            try
            {
                std::string polygonUrl = floodArea.value("polygon", "");
                // std::cout << "Fetching polygon from: " << polygonUrl;

                auto polygonData = fetchUrl(polygonUrl);
                if (polygonData)
                {
                    try
                    {
                        json polygonJson = json::parse(*polygonData);
                        warning.floodAreaPolygon = parseGeoJsonPolygon(polygonJson["features"][0]["geometry"]);
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "Error parsing polygon from URL: " << e.what();
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing embedded polygon: " << e.what();
            }
        }
    }

    return warning;
}

MultiPolygon FloodWarning::parseGeoJsonPolygon(const json &geoJson)
{
    MultiPolygon result;
    auto type = geoJson.value("type", "");

    if (type == "Polygon")
    {
        // Single polygon: { "type": "Polygon", "coordinates": [[[lon,lat],...]] }
        if (geoJson.contains("coordinates") && geoJson["coordinates"].is_array())
        {
            MyPolygon polygon;
            for (const auto &ring : geoJson["coordinates"])
            {
                LinearRing linearRing;
                for (const auto &coord : ring)
                {
                    if (coord.is_array() && coord.size() >= 2)
                    {
                        double lon = coord[0].get<double>();
                        double lat = coord[1].get<double>();
                        linearRing.emplace_back(lon, lat);
                    }
                }
                if (!linearRing.empty())
                {
                    polygon.push_back(linearRing);
                }
            }
            if (!polygon.empty())
            {
                result.push_back(polygon);
            }
        }
    }
    else if (type == "MultiPolygon")
    {
        // MultiPolygon: { "type": "MultiPolygon", "coordinates": [[[[lon,lat],...]]] }
        if (geoJson.contains("coordinates") && geoJson["coordinates"].is_array())
        {
            for (const auto &polygonCoords : geoJson["coordinates"])
            {
                MyPolygon polygon;
                for (const auto &ring : polygonCoords)
                {
                    LinearRing linearRing;
                    for (const auto &coord : ring)
                    {
                        if (coord.is_array() && coord.size() >= 2)
                        {
                            double lon = coord[0].get<double>();
                            double lat = coord[1].get<double>();
                            linearRing.emplace_back(lon, lat);
                        }
                    }
                    if (!linearRing.empty())
                    {
                        polygon.push_back(linearRing);
                    }
                }
                if (!polygon.empty())
                {
                    result.push_back(polygon);
                }
            }
        }
    }
    else
    {
        std::cerr << "Error parsing polygon - Unknown structure";
    }

    return result;
};