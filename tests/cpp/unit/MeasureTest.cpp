// tests/unit/MeasureTest.cpp
#include "Measure.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using nlohmann::json;

TEST(MeasureFromJsonTest, DefaultBehaviour) {
  json j = {{"@id", "m1"},
            {"parameter", "p"},
            {"parameterName", "Param"},
            {"period", 1.5},
            {"qualifier", "Q"},
            {"unitName", "unit"},
            {"latestReading", json::object({{"value", 56.78}})}};

  Measure m = Measure::fromJson(j);

  EXPECT_EQ(m.getId(), "m1");
  EXPECT_EQ(m.getParameter(), "p");
  EXPECT_EQ(m.getParameterName(), "Param");
  EXPECT_DOUBLE_EQ(m.getPeriod(), 1.5);
  EXPECT_EQ(m.getQualifier(), "Q");
  EXPECT_EQ(m.getUnitName(), "unit");
  EXPECT_DOUBLE_EQ(m.getLatestReading(), 56.78);
}

TEST(MeasureFromJsonTest, DefaultsWhenFieldsMissing) {
  json j = json::object(); // empty

  Measure m = Measure::fromJson(j);

  EXPECT_EQ(m.getId(), "");
  EXPECT_EQ(m.getParameter(), "");
  EXPECT_EQ(m.getParameterName(), "");
  EXPECT_EQ(m.getQualifier(), "");
  EXPECT_EQ(m.getUnitName(), "");
  EXPECT_DOUBLE_EQ(m.getPeriod(), 0.0);
  EXPECT_DOUBLE_EQ(m.getLatestReading(), 0.0);
}

TEST(MeasureFromJsonTest, LatestReadingObjectWithoutValueLeavesDefault) {
  json j = {{"@id", "m3"},
            {"latestReading", json::object({{"timestamp", "2025-01-01T00:00:00Z"}})}};

  Measure m = Measure::fromJson(j);

  EXPECT_EQ(m.getId(), "m3");
  EXPECT_DOUBLE_EQ(m.getLatestReading(), 0.0); // should remain default
}

TEST(MeasureFromJsonExtra, LatestReadingNullKeepsDefault) {
  json j = {{"@id", "null"}, {"latestReading", nullptr}};

  EXPECT_NO_THROW({
    Measure m = Measure::fromJson(j);
    EXPECT_EQ(m.getId(), "null");
    EXPECT_DOUBLE_EQ(m.getLatestReading(), 0.0); // should remain default
  });
}

TEST(MeasureFromJsonExtra, LatestReadingObjectWithNonNumericValueThrows) {
  json j = {{"@id", "badvalue"}, {"latestReading", json::object({{"value", "not-a-number"}})}};

  // fromJson calls get<double>() here and will throw.
  EXPECT_THROW({ Measure::fromJson(j); }, nlohmann::json::type_error);
}
