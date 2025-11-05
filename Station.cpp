#include <iostream>
#include <curl/curl.h>

#include "Station.hpp"
#include "HttpClient.hpp"

Station Station::fromJson(const json& jsonObj) {
    Station station;
    station.RLOIid = jsonObj.value("RLOIid", "unknown");
    station.catchmentName = jsonObj.value("catchmentName", "unknown");
    station.dateOpened = jsonObj.value("dateOpened", "unknown");
    station.label = jsonObj.value("label", "unknown");
    station.lat = jsonObj.value("lat", 0.0);
    station.lon = jsonObj.value("long", 0.0);
    station.northing = jsonObj.value("northing", 0L);
    station.easting = jsonObj.value("easting", 0L);
    station.notation = jsonObj.value("notation", "unknown");
    station.town = jsonObj.value("town", "unknown");
    station.riverName = jsonObj.value("riverName", "unknown");

    if (jsonObj.contains("measures") && jsonObj["measures"].is_array()) {
        for (const auto& measureJson : jsonObj["measures"]) {
            station.measures.push_back(Measure::fromJson(measureJson));
        }
    }
    
    return station;
}
