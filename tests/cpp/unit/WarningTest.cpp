// tests/unit/WarningTest.cpp
#include "Warning.hpp"
#include <gtest/gtest.h>
#include <simdjson.h>

TEST(WarningFromJsonTest, DefaultBehaviour) {
  std::string jsonStr = R"({
    "floodAreaID": "m1",
    "description": "long description",
    "eaAreaName": "a",
    "severity": "s",
    "severityLevel": 0,
    "timeMessageChanged": "2025-12-03T12:44:00",
    "timeRaised": "2025-12-03T12:44:34",
    "timeSeverityChanged": "2025-11-30T18:55:00",
    "message": "very long message!",
    "floodArea": {
      "county": "c",
      "polygon": "m2"
    }
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Warning fw = Warning::fromJson(json);

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

TEST(WarningFromJsonTest, DefaultsWhenFieldsMissing) {
  std::string jsonStr = "{}";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Warning fw = Warning::fromJson(json);

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

TEST(WarningFromJsonTest, FloodAreaNotAnObject) {
  std::string jsonStr = R"({
    "floodAreaID": "m1",
    "description": "long description",
    "floodArea": null
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Warning fw = Warning::fromJson(json);

  EXPECT_EQ(fw.getId(), "m1");
  EXPECT_EQ(fw.getDescription(), "long description");
  EXPECT_EQ(fw.getCounty(), "unknown");
  EXPECT_EQ(fw.getPolygonUrl(), "");
}

TEST(WarningParseGeoJsonPolygonTest, PolygonType) {
  std::string jsonStr = R"({
    "type": "Polygon",
    "coordinates": [
      [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto multiPolygon = Warning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 1);
  EXPECT_EQ(multiPolygon[0].size(), 1);
  EXPECT_EQ(multiPolygon[0][0].size(), 5);
}

TEST(WarningParseGeoJsonPolygonTest, MultiPolygonType) {
  std::string jsonStr = R"({
    "type": "MultiPolygon",
    "coordinates": [
      [[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]],
      [[[10.0, 10.0], [11.0, 10.0], [11.0, 11.0], [10.0, 11.0], [10.0, 10.0]]]
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto multiPolygon = Warning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 2);
  EXPECT_EQ(multiPolygon[0].size(), 1);
  EXPECT_EQ(multiPolygon[1].size(), 1);
}

TEST(WarningParseGeoJsonPolygonTest, UnknownType) {
  std::string jsonStr = R"({
    "type": "Point",
    "coordinates": []
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto result = Warning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result.size(), 0);
}

TEST(WarningParseGeoJsonPolygonTest, NoCoordinates) {
  std::string jsonStr = R"({"type": "Polygon"})";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto multiPolygon = Warning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 0);
}

TEST(WarningParseGeoJsonPolygonTest, CoordinatesArrayEmpty) {
  std::string jsonStr = R"({
    "type": "Polygon",
    "coordinates": []
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto result = Warning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result.size(), 0);
}

TEST(WarningParseGeoJsonPolygonTest, CoordinatesNotArray) {
  std::string jsonStr = R"({
    "type": "MultiPolygon",
    "coordinates": 69
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto multiPolygon = Warning::parseGeoJsonPolygon(geoJson);

  EXPECT_EQ(multiPolygon.size(), 0);
}

TEST(WarningParseGeoJsonPolygonTest, CoordinatesInvalid) {
  std::string jsonStr = R"({
    "type": "Polygon",
    "coordinates": [
      [[0.0, 1.0], "not an array", [2.0], null]
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto result = Warning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result[0][0].size(), 1);
}

TEST(WarningParseGeoJsonPolygonTest, CoordinatesAllInvalid) {
  std::string jsonStr = R"({
    "type": "Polygon",
    "coordinates": [
      ["invalid", [1.0], null]
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element geoJson;
  auto error = parser.parse(jsonStr).get(geoJson);
  ASSERT_EQ(error, 0U);

  auto result = Warning::parseGeoJsonPolygon(geoJson);
  EXPECT_EQ(result.size(), 0);
}
