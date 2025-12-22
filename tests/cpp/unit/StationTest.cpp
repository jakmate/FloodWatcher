// tests/unit/StationTest.cpp
#include "Station.hpp"
#include <gtest/gtest.h>
#include <simdjson.h>

TEST(StationFromJsonTest, DefaultBehaviour) {
  std::string jsonStr = R"({
    "RLOIid": "id",
    "catchmentName": "name",
    "dateOpened": "date",
    "label": "l",
    "lat": 51.874767,
    "long": -1.740083,
    "northing": 219610,
    "easting": 528000,
    "notation": "n",
    "town": "t",
    "riverName": "r",
    "status": "url/statusActive"
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Station m = Station::fromJson(json);

  EXPECT_EQ(m.getRLOIid(), "id");
  EXPECT_EQ(m.getCatchmentName(), "name");
  EXPECT_EQ(m.getDateOpened(), "date");
  EXPECT_EQ(m.getLabel(), "l");
  EXPECT_EQ(m.getLat(), 51.874767);
  EXPECT_EQ(m.getLon(), -1.740083);
  EXPECT_EQ(m.getNorthing(), 219610);
  EXPECT_EQ(m.getEasting(), 528000);
  EXPECT_EQ(m.getNotation(), "n");
  EXPECT_EQ(m.getTown(), "t");
  EXPECT_EQ(m.getRiverName(), "r");
}

TEST(StationFromJsonTest, DefaultsWhenFieldsMissing) {
  std::string jsonStr = "{}";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Station m = Station::fromJson(json);

  EXPECT_EQ(m.getRLOIid(), "unknown");
  EXPECT_EQ(m.getCatchmentName(), "unknown");
  EXPECT_EQ(m.getDateOpened(), "unknown");
  EXPECT_EQ(m.getLabel(), "unknown");
  EXPECT_EQ(m.getLat(), 0.0);
  EXPECT_EQ(m.getLon(), 0.0);
  EXPECT_EQ(m.getNorthing(), 0);
  EXPECT_EQ(m.getEasting(), 0);
  EXPECT_EQ(m.getNotation(), "unknown");
  EXPECT_EQ(m.getTown(), "unknown");
  EXPECT_EQ(m.getRiverName(), "unknown");
}

TEST(StationFromJsonTest, HandlesArrayFieldsWithActiveStatus) {
  std::string jsonStr = R"({
    "RLOIid": ["10427", "9154"],
    "catchmentName": ["Loddon", "London"],
    "dateOpened": ["date", "date2"],
    "label": ["Erith Deep Wharf", "Erith Deep Wharf TL"],
    "lat": [51.5, 52.5],
    "long": [-1.5, -2.5],
    "northing": [100000, 200000],
    "easting": [300000, 400000],
    "status": ["url/statusActive", "url/statusSuspended"],
    "notation": "0018"
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Station s = Station::fromJson(json);

  // Should pick first element (index 0) matching statusActive
  EXPECT_EQ(s.getRLOIid(), "10427");
  EXPECT_EQ(s.getCatchmentName(), "Loddon");
  EXPECT_EQ(s.getDateOpened(), "date");
  EXPECT_EQ(s.getLabel(), "Erith Deep Wharf");
  EXPECT_EQ(s.getLat(), 51.5);
  EXPECT_EQ(s.getLon(), -1.5);
  EXPECT_EQ(s.getNorthing(), 100000);
  EXPECT_EQ(s.getEasting(), 300000);
  EXPECT_EQ(s.getNotation(), "0018");
}

TEST(StationFromJsonTest, HandlesArrayFieldsWithoutStatus) {
  std::string jsonStr = R"({
    "RLOIid": ["10427", "9154"],
    "catchmentName": ["Loddon", "London"]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Station s = Station::fromJson(json);

  // Should fallback to first element when no status array
  EXPECT_EQ(s.getRLOIid(), "10427");
  EXPECT_EQ(s.getCatchmentName(), "Loddon");
}

TEST(StationFromJsonTest, HandlesArrayFieldsWithNoActiveStatus) {
  std::string jsonStr = R"({
    "RLOIid": ["10427", "9154"],
    "status": ["url/statusSuspended", "url/statusInactive"]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Station s = Station::fromJson(json);

  // Should fallback to first element when no statusActive found
  EXPECT_EQ(s.getRLOIid(), "10427");
}

TEST(StationFromJsonTest, HandlesStatusNotArray) {
  std::string jsonStr = R"({
    "RLOIid": ["10427", "9154"],
    "status": "url/statusActive"
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Station s = Station::fromJson(json);

  EXPECT_EQ(s.getRLOIid(), "10427"); // Fallback to first
}
