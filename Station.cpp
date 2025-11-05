#include <iostream>
#include <curl/curl.h>

#include "Station.hpp"
#include "HttpClient.hpp"

Station Station::fromJson(const json& jsonObj) {
    Station station;
    if (jsonObj.contains("RLOIid")) {
        station.RLOIid = jsonObj["RLOIid"].get<std::string>();
    } else {
        station.RLOIid = "unknown";
    }
    
    if (jsonObj.contains("catchmentName")) {
        station.catchmentName = jsonObj["catchmentName"].get<std::string>();
    } else {
        station.catchmentName = "unknown";
    }
    
    if (jsonObj.contains("dateOpened")) {
        station.dateOpened = jsonObj["dateOpened"].get<std::string>();
    } else {
        station.dateOpened = "unknown";
    }
    if (jsonObj.contains("lat")) {
        station.lat = jsonObj["lat"].get<double>();
    } else {
        station.lat = 0.0;
    }
    
    if (jsonObj.contains("long")) {
        station.lon = jsonObj["long"].get<double>();
    }  else {
        station.lon = 0.0;
    }

    if (jsonObj.contains("northing")) {
        station.northing = jsonObj["northing"].get<double>();
    } else {
        station.northing = 0.0;
    }
    
    if (jsonObj.contains("easting")) {
        station.easting = jsonObj["easting"].get<double>();
    }  else {
        station.easting = 0.0;
    }
    
    if (jsonObj.contains("notation")) {
        station.notation = jsonObj["notation"].get<std::string>();
    } else {
        station.notation = "unknown";
    }
    
    if (jsonObj.contains("town")) {
        station.town = jsonObj["town"].get<std::string>();
    } else {
        station.town = "unknown";
    }
    
    return station;
}
