// tests/unit/MonitoringDataTest.cpp
#include "MonitoringData.hpp"
#include "MockHttpClient.hpp"
#include <HttpClient.hpp>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST(ParseWarningsTest, ParsesValidItemsArray) {
  json apiResponse = {
      {"items", json::array({{{"severityLevel", 1}, {"message", "Flood alert"}},
                             {{"severityLevel", 2}, {"message", "Flood warning"}},
                             {{"severityLevel", 3}, {"message", "Severe flood warning"}}})}};

  MonitoringData data;
  data.parseWarnings(apiResponse);

  EXPECT_EQ(data.getWarnings().size(), 3);
  EXPECT_EQ(data.getWarnings()[0].getMessage(), "Flood alert");
  EXPECT_EQ(data.getWarnings()[1].getSeverityLevel(), 2);
}

TEST(ParseWarningsTest, HandlesEmptyItems) {
  json apiResponse = {{"items", json::array()}};

  MonitoringData data;
  data.parseWarnings(apiResponse);

  EXPECT_TRUE(data.getWarnings().empty());
}

TEST(ParseWarningsTest, HandlesMissingItemsKey) {
  json apiResponse = json::object();

  MonitoringData data;
  data.parseWarnings(apiResponse);

  EXPECT_TRUE(data.getWarnings().empty());
}

TEST(ParseWarningsTest, CatchesInvalidJsonAndContinues) {
  json apiResponse = {
      {"items", json::array({{{"severityLevel", 1}, {"message", "Valid warning"}},
                             {{"severityLevel", "invalid"}}, // Wrong type
                             nullptr,                        // Null item
                             {{"severityLevel", 2}, {"message", "Another valid"}}})}};

  MonitoringData data;

  // Should not throw - catches internally
  EXPECT_NO_THROW(data.parseWarnings(apiResponse));

  // Should parse only valid items
  EXPECT_EQ(data.getWarnings().size(), 2);
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

  MonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_EQ(data.getStations().size(), 3);
  EXPECT_EQ(data.getStations()[0].getRLOIid(), "station1");
  EXPECT_EQ(data.getStations()[1].getLabel(), "Test Station 2");
}

TEST(ParseStationsTest, HandlesEmptyItems) {
  json apiResponse = {{"items", json::array()}};

  MonitoringData data;
  data.parseStations(apiResponse);

  EXPECT_TRUE(data.getStations().empty());
}

TEST(ParseStationsTest, HandlesMissingItemsKey) {
  json apiResponse = json::object();

  MonitoringData data;
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

  MonitoringData data;

  // Should not throw - catches internally
  EXPECT_NO_THROW(data.parseStations(apiResponse));

  // Should parse only valid items
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
      json testWarning1 = R"({
            "floodAreaID": "test1",
            "description": "Test warning 1",
            "eaAreaName": "Test Area 1",
            "severity": "Moderate",
            "severityLevel": 2,
            "floodArea": {
                "county": "Test County",
                "polygon": "http://test-polygon1.com"
            }
        })"_json;

      json testWarning2 = R"({
            "floodAreaID": "test2",
            "description": "Test warning 2",
            "eaAreaName": "Test Area 2",
            "severity": "Severe",
            "severityLevel": 3,
            "floodArea": {
                "county": "Test County",
                "polygon": "http://test-polygon2.com"
            }
        })"_json;

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
  EXPECT_EQ(poly1.size(), 1);       // One polygon
  EXPECT_EQ(poly1[0].size(), 1);    // One ring
  EXPECT_EQ(poly1[0][0].size(), 5); // Closed ring with 5 points

  // Verify second warning got its polygon
  auto opt2 = warnings[1].getFloodAreaPolygon();
  ASSERT_TRUE(opt2.has_value());
  const auto& poly2 = opt2.value(); // NOLINT(bugprone-unchecked-optional-access)
  EXPECT_EQ(poly2.size(), 1);
}

TEST_F(MonitoringDataTest, FetchAllPolygonsAsync_HandlesFailedRequests) {
  // Setup one failing response
  getMockClient().addResponse("http://test-polygon1.com", "invalid json data");
  // polygon2 still works

  // Act
  getData().fetchAllPolygonsAsync();

  // Assert
  const auto& warnings = getData().getWarnings();

  // First warning should fail to parse
  EXPECT_FALSE(warnings[0].getFloodAreaPolygon().has_value());

  // Second warning should succeed
  auto opt2b = warnings[1].getFloodAreaPolygon();
  ASSERT_TRUE(opt2b.has_value());
}