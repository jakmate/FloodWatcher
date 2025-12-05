// tests/unit/FloodWarningDataTest.cpp
#include "FloodMonitoringData.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST(ParseFloodWarningsTest, ParsesValidItemsArray) {
  json apiResponse = {
      {"items", json::array({{{"severityLevel", 1}, {"message", "Flood alert"}},
                             {{"severityLevel", 2}, {"message", "Flood warning"}},
                             {{"severityLevel", 3}, {"message", "Severe flood warning"}}})}};

  FloodMonitoringData data;
  data.parseFloodWarnings(apiResponse);

  EXPECT_EQ(data.getFloodWarnings().size(), 3);
  EXPECT_EQ(data.getFloodWarnings()[0].getMessage(), "Flood alert");
  EXPECT_EQ(data.getFloodWarnings()[1].getSeverityLevel(), 2);
}

TEST(ParseFloodWarningsTest, HandlesEmptyItems) {
  json apiResponse = {{"items", json::array()}};

  FloodMonitoringData data;
  data.parseFloodWarnings(apiResponse);

  EXPECT_TRUE(data.getFloodWarnings().empty());
}

TEST(ParseFloodWarningsTest, HandlesMissingItemsKey) {
  json apiResponse = json::object();

  FloodMonitoringData data;
  data.parseFloodWarnings(apiResponse);

  EXPECT_TRUE(data.getFloodWarnings().empty());
}

TEST(ParseFloodWarningsTest, CatchesInvalidJsonAndContinues) {
  json apiResponse = {
      {"items", json::array({{{"severityLevel", 1}, {"message", "Valid warning"}},
                             {{"severityLevel", "invalid"}}, // Wrong type
                             nullptr,                        // Null item
                             {{"severityLevel", 2}, {"message", "Another valid"}}})}};

  FloodMonitoringData data;

  // Should not throw - catches internally
  EXPECT_NO_THROW(data.parseFloodWarnings(apiResponse));

  // Should parse only valid items
  EXPECT_EQ(data.getFloodWarnings().size(), 2);
}

TEST(ParseStationsTest, ParsesValidItemsArray) {
  json apiResponse = {
      {"items",
       json::array(
           {{{"RLOIid", "station1"}, {"label", "Test Station 1"}, {"lat", 51.5}, {"long", -0.1}},
            {{"RLOIid", "station2"}, {"label", "Test Station 2"}, {"lat", 52.0}, {"long", -1.0}},
            {{"RLOIid", "station3"},
             {"label", "Test Station 3"},
             {"lat", 53.5},
             {"long", -2.5}}})}};

  FloodMonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_EQ(data.getStations().size(), 3);
  EXPECT_EQ(data.getStations()[0].getRLOIid(), "station1");
  EXPECT_EQ(data.getStations()[1].getLabel(), "Test Station 2");
}

TEST(ParseStationsTest, HandlesEmptyItems) {
  json apiResponse = {{"items", json::array()}};

  FloodMonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_TRUE(data.getStations().empty());
}

TEST(ParseStationsTest, HandlesMissingItemsKey) {
  json apiResponse = json::object();

  FloodMonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_TRUE(data.getStations().empty());
}

TEST(ParseStationsTest, CatchesInvalidJsonAndContinues) {
  json apiResponse = {
      {"items",
       json::array(
           {{{"RLOIid", "valid station"}, {"lat", 51.5}, {"long", -0.1}},
            {{"RLOIid", "wrong type station"}, {"lat", "48.0"}, {"long", -0.1}}, // Wrong type
            nullptr,                                                             // Null item
            {{"RLOIid", "another valid station"}, {"lat", 50.5}, {"long", -0.2}}})}};

  FloodMonitoringData data;

  // Should not throw - catches internally
  EXPECT_NO_THROW(data.parseStations(apiResponse));

  // Should parse only valid items
  EXPECT_EQ(data.getStations().size(), 2);
}
