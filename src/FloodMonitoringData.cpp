#include <iostream>
#include "FloodMonitoringData.hpp"

void FloodMonitoringData::parseFloodWarnings(const json &apiResponse)
{
    if (apiResponse.contains("items") && apiResponse["items"].is_array())
    {
        for (const auto &item : apiResponse["items"])
        {
            try
            {
                floodWarnings.push_back(FloodWarning::fromJson(item));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing flood warning: " << e.what() << std::endl;
            }
        }
    }
}

void FloodMonitoringData::parseStations(const json &apiResponse)
{
    if (apiResponse.contains("items") && apiResponse["items"].is_array())
    {
        for (const auto &item : apiResponse["items"])
        {
            try
            {
                stations.push_back(Station::fromJson(item));
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error parsing station: " << e.what() << std::endl;
            }
        }
    }
}