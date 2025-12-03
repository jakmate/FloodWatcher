// tests/unit/StationTest.cpp
#include "Station.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST(StationFromJsonTest, DefaultBehaviour) {
  json j = {{"RLOIid", "id"},     {"catchmentName", "name"}, {"dateOpened", "date"},
            {"label", "l"},       {"lat", 51.874767},        {"long", -1.740083},
            {"northing", 219610}, {"easting", 528000},       {"notation", "n"},
            {"town", "t"},        {"riverName", "r"}};

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
