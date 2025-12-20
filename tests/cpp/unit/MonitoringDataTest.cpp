// tests/unit/MonitoringDataTest.cpp
#include "MonitoringData.hpp"
#include "MockHttpClient.hpp"
#include <HttpClient.hpp>
#include <gtest/gtest.h>
#include <simdjson.h>

TEST(ParseWarningsTest, ParsesValidItemsArray) {
  std::string jsonStr = R"({
    "items": [
      {"severityLevel": 1, "message": "Flood alert"},
      {"severityLevel": 2, "message": "Flood warning"},
      {"severityLevel": 3, "message": "Severe flood warning"}
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;
  data.parseWarnings(apiResponse);

  EXPECT_EQ(data.getWarnings().size(), 3);
  EXPECT_EQ(data.getWarnings()[0].getMessage(), "Flood alert");
  EXPECT_EQ(data.getWarnings()[1].getSeverityLevel(), 2);
}

TEST(ParseWarningsTest, HandlesEmptyItems) {
  std::string jsonStr = R"({"items": []})";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;
  data.parseWarnings(apiResponse);

  EXPECT_TRUE(data.getWarnings().empty());
}

TEST(ParseWarningsTest, HandlesMissingItemsKey) {
  std::string jsonStr = "{}";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;
  data.parseWarnings(apiResponse);

  EXPECT_TRUE(data.getWarnings().empty());
}

TEST(ParseWarningsTest, CatchesInvalidJsonAndContinues) {
  std::string jsonStr = R"({
    "items": [
      {"severityLevel": 1, "message": "Valid warning"},
      {"severityLevel": "invalid"},
      null,
      {"severityLevel": 2, "message": "Another valid"}
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;

  EXPECT_NO_THROW(data.parseWarnings(apiResponse));
  EXPECT_EQ(data.getWarnings().size(), 2);
}

TEST(ParseStationsTest, ParsesValidItemsArray) {
  std::string jsonStr = R"({
    "items": [
      {"RLOIid": "station1", "label": "Test Station 1", "lat": 51.5, "long": -0.1},
      {"RLOIid": "station2", "label": "Test Station 2", "lat": 52.0, "long": -1.0},
      {"RLOIid": "station3", "label": "Test Station 3", "lat": 53.5, "long": -2.5}
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_EQ(data.getStations().size(), 3);
  EXPECT_EQ(data.getStations()[0].getRLOIid(), "station1");
  EXPECT_EQ(data.getStations()[1].getLabel(), "Test Station 2");
}

TEST(ParseStationsTest, HandlesEmptyItems) {
  std::string jsonStr = R"({"items": []})";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_TRUE(data.getStations().empty());
}

TEST(ParseStationsTest, HandlesMissingItemsKey) {
  std::string jsonStr = "{}";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_TRUE(data.getStations().empty());
}

TEST(ParseStationsTest, CatchesInvalidJsonAndContinues) {
  std::string jsonStr = R"({
    "items": [
      {"RLOIid": "valid station", "lat": 51.5, "long": -0.1},
      {"RLOIid": "wrong type station", "lat": "48.0", "long": -0.1},
      null,
      {"RLOIid": "another valid station", "lat": 50.5, "long": -0.2}
    ]
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element apiResponse;
  auto error = parser.parse(jsonStr).get(apiResponse);
  ASSERT_EQ(error, 0U);

  MonitoringData data;

  EXPECT_NO_THROW(data.parseStations(apiResponse));
  EXPECT_EQ(data.getStations().size(), 2);
}

class MonitoringDataTest : public ::testing::Test {
  private:
    MockHttpClient mockClient;
    MonitoringData data;

  protected:
    MonitoringData& getData() {
      return data;
    }
    MockHttpClient& getMockClient() {
      return mockClient;
    }

    void SetUp() override {
      HttpClient::setInstance(&mockClient);

      std::string warning1Str = R"({
        "floodAreaID": "test1",
        "description": "Test warning 1",
        "eaAreaName": "Test Area 1",
        "severity": "Moderate",
        "severityLevel": 2,
        "floodArea": {
          "county": "Test County",
          "polygon": "http://test-polygon1.com"
        }
      })";

      std::string warning2Str = R"({
        "floodAreaID": "test2",
        "description": "Test warning 2",
        "eaAreaName": "Test Area 2",
        "severity": "Severe",
        "severityLevel": 3,
        "floodArea": {
          "county": "Test County",
          "polygon": "http://test-polygon2.com"
        }
      })";

      simdjson::dom::parser parser;
      simdjson::dom::element testWarning1;
      simdjson::dom::element testWarning2;

      auto error1 = parser.parse(warning1Str).get(testWarning1);
      ASSERT_EQ(error1, 0U);
      auto error2 = parser.parse(warning2Str).get(testWarning2);
      ASSERT_EQ(error2, 0U);

      auto warning1 = Warning::fromJson(testWarning1);
      auto warning2 = Warning::fromJson(testWarning2);

      data.warnings = {warning1, warning2};

      // Setup mock responses
      mockClient.addResponse("http://test-polygon1.com", R"({
        "features": [{
          "geometry": {
            "type": "Polygon",
            "coordinates": [[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]]
          }
        }]
      })");

      mockClient.addResponse("http://test-polygon2.com", R"({
        "features": [{
          "geometry": {
            "type": "Polygon",
            "coordinates": [[[2.0, 2.0], [3.0, 2.0], [3.0, 3.0], [2.0, 3.0], [2.0, 2.0]]]
          }
        }]
      })");
    }

    void TearDown() override {
      HttpClient::setInstance(nullptr);
    }
};

TEST_F(MonitoringDataTest, FetchAllPolygonsAsync_CompletesSuccessfully) {
  // Act
  getData().fetchAllPolygonsAsync();

  // Assert
  const auto& warnings = getData().getWarnings();
  ASSERT_EQ(warnings.size(), 2);

  // Verify first warning got its polygon
  auto opt1 = warnings[0].getFloodAreaPolygon();
  ASSERT_TRUE(opt1.has_value());
  const auto& poly1 = opt1.value(); // NOLINT(bugprone-unchecked-optional-access)
  EXPECT_EQ(poly1.size(), 1);
  EXPECT_EQ(poly1[0].size(), 1);
  EXPECT_EQ(poly1[0][0].size(), 5);

  // Verify second warning got its polygon
  auto opt2 = warnings[1].getFloodAreaPolygon();
  ASSERT_TRUE(opt2.has_value());
  const auto& poly2 = opt2.value(); // NOLINT(bugprone-unchecked-optional-access)
  EXPECT_EQ(poly2.size(), 1);
}
