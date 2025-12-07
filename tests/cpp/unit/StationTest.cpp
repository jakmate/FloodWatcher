// tests/unit/StationTest.cpp
#include "Station.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST(StationFromJsonTest, DefaultBehaviour) {
  json j = {{"RLOIid", "id"},     {"catchmentName", "name"}, {"dateOpened", "date"},
            {"label", "l"},       {"lat", 51.874767},        {"long", -1.740083},
            {"northing", 219610}, {"easting", 528000},       {"notation", "n"},
            {"town", "t"},        {"riverName", "r"},        {"status", "url/statusActive"}};

  Station m = Station::fromJson(j);

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
  json j = json::object(); // empty

  Station m = Station::fromJson(j);

  EXPECT_EQ(m.getRLOIid(), "unknown");
  EXPECT_EQ(m.getCatchmentName(), "unknown");
  EXPECT_EQ(m.getDateOpened(), "unknown");
  EXPECT_EQ(m.getLabel(), "unknown");
  EXPECT_EQ(m.getLat(), 0.0);
  EXPECT_EQ(m.getLon(), 0.0);
  EXPECT_EQ(m.getNorthing(), 0L);
  EXPECT_EQ(m.getEasting(), 0L);
  EXPECT_EQ(m.getNotation(), "unknown");
  EXPECT_EQ(m.getTown(), "unknown");
  EXPECT_EQ(m.getRiverName(), "unknown");
}

TEST(StationFromJsonTest, HandlesArrayFieldsWithActiveStatus) {
  json j = {{"RLOIid", json::array({"10427", "9154"})},
            {"catchmentName", json::array({"Loddon", "London"})},
            {"dateOpened", json::array({"date", "date2"})},
            {"label", json::array({"Erith Deep Wharf", "Erith Deep Wharf TL"})},
            {"lat", json::array({51.5, 52.5})},
            {"long", json::array({-1.5, -2.5})},
            {"northing", json::array({100000, 200000})},
            {"easting", json::array({300000, 400000})},
            {"status", json::array({"url/statusActive", "url/statusSuspended"})},
            {"notation", "0018"}};

  Station s = Station::fromJson(j);

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
  json j = {{"RLOIid", json::array({"10427", "9154"})},
            {"catchmentName", json::array({"Loddon", "London"})}};

  Station s = Station::fromJson(j);

  // Should fallback to first element when no status array
  EXPECT_EQ(s.getRLOIid(), "10427");
  EXPECT_EQ(s.getCatchmentName(), "Loddon");
}

TEST(StationFromJsonTest, HandlesArrayFieldsWithNoActiveStatus) {
  json j = {{"RLOIid", json::array({"10427", "9154"})},
            {"status", json::array({"url/statusSuspended", "url/statusInactive"})}};

  Station s = Station::fromJson(j);

  // Should fallback to first element when no statusActive found
  EXPECT_EQ(s.getRLOIid(), "10427");
}

TEST(StationFromJsonTest, HandlesStatusNotArray) {
  json j = {{"RLOIid", json::array({"10427", "9154"})},
            {"status", "url/statusActive"}};

  Station s = Station::fromJson(j);

  EXPECT_EQ(s.getRLOIid(), "10427");  // Fallback to first
}
