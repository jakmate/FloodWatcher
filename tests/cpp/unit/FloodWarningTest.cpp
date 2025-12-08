// tests/unit/FloodWarningTest.cpp
#include "FloodWarning.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST(FloodWarningFromJsonTest, DefaultBehaviour) {
  json j = {{"floodAreaID", "m1"},
            {"description", "long description"},
            {"eaAreaName", "a"},
            {"severity", "s"},
            {"severityLevel", 0},
            {"timeMessageChanged", "2025-12-03T12:44:00"},
            {"timeRaised", "2025-12-03T12:44:34"},
            {"timeSeverityChanged", "2025-11-30T18:55:00"},
            {"message", "very long message!"},
            {"floodArea", json::object({{"county", "c"}, {"polygon", "m2"}})}};

  FloodWarning fw = FloodWarning::fromJson(j);

  EXPECT_EQ(fw.getId(), "m1");
  EXPECT_EQ(fw.getDescription(), "long description");
  EXPECT_EQ(fw.getAreaName(), "a");
  EXPECT_EQ(fw.getSeverity(), "s");
  EXPECT_EQ(fw.getSeverityLevel(), 0);
  EXPECT_EQ(fw.getTimeMessageChanged(), "2025-12-03T12:44:00");
  EXPECT_EQ(fw.getTimeRaised(), "2025-12-03T12:44:34");
  EXPECT_EQ(fw.getTimeSeverityChanged(), "2025-11-30T18:55:00");
  EXPECT_EQ(fw.getMessage(), "very long message!");
  EXPECT_EQ(fw.getCounty(), "c");
  EXPECT_EQ(fw.getPolygonUrl(), "m2");
}

TEST(FloodWarningFromJsonTest, DefaultsWhenFieldsMissing) {
  json j = json::object(); // empty

  FloodWarning fw = FloodWarning::fromJson(j);

  EXPECT_EQ(fw.getId(), "unknown");
  EXPECT_EQ(fw.getDescription(), "unknown");
  EXPECT_EQ(fw.getAreaName(), "unknown");
  EXPECT_EQ(fw.getSeverity(), "unknown");
  EXPECT_EQ(fw.getSeverityLevel(), 0);
  EXPECT_EQ(fw.getTimeMessageChanged(), "");
  EXPECT_EQ(fw.getTimeRaised(), "");
  EXPECT_EQ(fw.getTimeSeverityChanged(), "");
  EXPECT_EQ(fw.getMessage(), "");
  EXPECT_EQ(fw.getCounty(), "");
  EXPECT_EQ(fw.getPolygonUrl(), "");
}

TEST(FloodWarningFromJsonTest, FloodAreaNotAnObject) {
  json j = {{"floodAreaID", "m1"}, {"description", "long description"}, {"floodArea", nullptr}};

  FloodWarning fw = FloodWarning::fromJson(j);

  EXPECT_EQ(fw.getId(), "m1");
  EXPECT_EQ(fw.getDescription(), "long description");
  EXPECT_EQ(fw.getCounty(), "");
  EXPECT_EQ(fw.getPolygonUrl(), "");
}

TEST(FloodWarningParseGeoJsonPolygonTest, PolygonType) {
  json geoJson = {
      {"type", "Polygon"},
      {"coordinates", json::array({json::array({// Single ring
                                                json::array({0.0, 0.0}), json::array({1.0, 0.0}),
                                                json::array({1.0, 1.0}), json::array({0.0, 1.0}),
                                                json::array({0.0, 0.0})})})}};

  auto multiPolygon = FloodWarning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 1);       // One polygon
  EXPECT_EQ(multiPolygon[0].size(), 1);    // One ring in the polygon
  EXPECT_EQ(multiPolygon[0][0].size(), 5); // Five points in the ring
}

TEST(FloodWarningParseGeoJsonPolygonTest, MultiPolygonType) {
  json geoJson = {
      {"type", "MultiPolygon"},
      {"coordinates",
       json::array({json::array({// First polygon
                                 json::array({json::array({0.0, 0.0}), json::array({1.0, 0.0}),
                                              json::array({1.0, 1.0}), json::array({0.0, 1.0}),
                                              json::array({0.0, 0.0})})}),
                    json::array({// Second polygon
                                 json::array({json::array({10.0, 10.0}), json::array({11.0, 10.0}),
                                              json::array({11.0, 11.0}), json::array({10.0, 11.0}),
                                              json::array({10.0, 10.0})})})})}};

  auto multiPolygon = FloodWarning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 2);    // Two polygons
  EXPECT_EQ(multiPolygon[0].size(), 1); // One ring in first polygon
  EXPECT_EQ(multiPolygon[1].size(), 1); // One ring in second polygon
}

TEST(FloodWarningParseGeoJsonPolygonTest, UnknownType) {
  json geoJson = {{"type", "Point"}, {"coordinates", json::array()}};
  auto result = FloodWarning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result.size(), 0);
}

TEST(FloodWarningParseGeoJsonPolygonTest, NoCoordinates) {
  json geoJson = {{"type", "Polygon"}};

  auto multiPolygon = FloodWarning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 0);
}

TEST(FloodWarningParseGeoJsonPolygonTest, CoordinatesArrayEmpty) {
  json geoJson = {{"type", "Polygon"}, {"coordinates", json::array()}};
  auto result = FloodWarning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result.size(), 0);
}

TEST(FloodWarningParseGeoJsonPolygonTest, CoordinatesNotArray) {
  json geoJson = {{"type", "MultiPolygon"}, {"coordinates", 69}};

  auto multiPolygon = FloodWarning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 0);
}

TEST(FloodWarningParseGeoJsonPolygonTest, CoordinatesInvalid) {
  json geoJson = {
      {"type", "Polygon"},
      {"coordinates", json::array({json::array({json::array({0.0, 1.0}), // valid
                                                "not an array",          // invalid
                                                json::array({2.0}),      // invalid (< 2 elements)
                                                nullptr})})}};

  auto result = FloodWarning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result[0][0].size(), 1); // Only 1 valid coords
}

TEST(FloodWarningParseGeoJsonPolygonTest, CoordinatesAllInvalid) {
  json geoJson = {
      {"type", "Polygon"},
      {"coordinates", json::array({json::array({"invalid", json::array({1.0}), nullptr})})}};

  auto result = FloodWarning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result.size(), 0); // Empty polygon filtered out
}
