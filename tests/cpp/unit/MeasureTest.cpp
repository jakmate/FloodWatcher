// tests/unit/MeasureTest.cpp
#include "Measure.hpp"
#include <cmath>
#include <gtest/gtest.h>

TEST(MeasureFromJsonTest, DefaultBehaviour) {
  std::string jsonStr = R"({
    "@id": "m1",
    "parameter": "p",
    "parameterName": "Param",
    "period": 1.5,
    "qualifier": "Q",
    "unitName": "unit",
    "latestReading": {"value": 56.78}
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Measure m = Measure::fromJson(json);

  EXPECT_EQ(m.getId(), "m1");
  EXPECT_EQ(m.getParameter(), "p");
  EXPECT_EQ(m.getParameterName(), "Param");
  EXPECT_DOUBLE_EQ(m.getPeriod(), 1.5);
  EXPECT_EQ(m.getQualifier(), "Q");
  EXPECT_EQ(m.getUnitName(), "unit");
  EXPECT_DOUBLE_EQ(m.getLatestReading(), 56.78);
}

TEST(MeasureFromJsonTest, DefaultsWhenFieldsMissing) {
  std::string jsonStr = "{}";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Measure m = Measure::fromJson(json);

  EXPECT_EQ(m.getId(), "");
  EXPECT_EQ(m.getParameter(), "");
  EXPECT_EQ(m.getParameterName(), "");
  EXPECT_EQ(m.getQualifier(), "");
  EXPECT_EQ(m.getUnitName(), "");
  EXPECT_DOUBLE_EQ(m.getPeriod(), 0.0);
  EXPECT_DOUBLE_EQ(m.getLatestReading(), 0.0);
}

TEST(MeasureFromJsonTest, LatestReadingObjectWithoutValueLeavesDefault) {
  std::string jsonStr = R"({
    "@id": "m3",
    "latestReading": {"timestamp": "2025-01-01T00:00:00Z"}
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  Measure m = Measure::fromJson(json);

  EXPECT_EQ(m.getId(), "m3");
  EXPECT_DOUBLE_EQ(m.getLatestReading(), 0.0); // should remain default
}

TEST(MeasureFromJsonExtra, LatestReadingNullKeepsDefault) {
  std::string jsonStr = R"({
    "@id": "null",
    "latestReading": null
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  EXPECT_NO_THROW({
    Measure m = Measure::fromJson(json);
    EXPECT_EQ(m.getId(), "null");
    EXPECT_DOUBLE_EQ(m.getLatestReading(), 0.0); // should remain default
  });
}

TEST(MeasureFromJsonExtra, LatestReadingObjectWithNonNumericValue) {
  std::string jsonStr = R"({
    "@id": "badvalue",
    "latestReading": {"value": "not-a-number"}
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element json;
  auto error = parser.parse(jsonStr).get(json);
  ASSERT_EQ(error, 0U);

  // Should throw because value is not a number
  EXPECT_THROW(Measure::fromJson(json), std::runtime_error);
}
