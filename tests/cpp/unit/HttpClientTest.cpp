// tests/cpp/unit/HttpClientTest.cpp
#include "HttpClient.hpp"
#include <gtest/gtest.h>
#include <thread>

TEST(HttpClientTest, FetchValidUrl) {
  auto result = HttpClient::fetchUrl("https://www.google.com/");

  ASSERT_TRUE(result.has_value());
  if (result.has_value()) {
    EXPECT_FALSE(result->empty());
  }
}

TEST(HttpClientTest, Fetch404ReturnsNullopt) {
  auto result = HttpClient::fetchUrl("https://www.google.com/error");

  EXPECT_FALSE(result.has_value());
}

TEST(HttpClientTest, InvalidUrlReturnsNullopt) {
  auto result = HttpClient::fetchUrl("invalid://url");

  EXPECT_FALSE(result.has_value());
}

TEST(HttpClientTest, SingletonReturnsSameInstance) {
  HttpClient& instance1 = HttpClient::getInstance();
  HttpClient& instance2 = HttpClient::getInstance();

  EXPECT_EQ(&instance1, &instance2);
}

TEST(HttpClientTest, MultipleRequestsWork) {
  auto result1 = HttpClient::fetchUrl("https://www.google.com/");
  auto result2 = HttpClient::fetchUrl("https://www.google.com/");

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
}

TEST(HttpClientTest, ThreadSafety) {
  std::vector<std::thread> threads;
  threads.reserve(5);
  std::atomic<int> successCount{0};

  for (int i = 0; i < 5; ++i) {
    threads.emplace_back([&successCount]() {
      auto result = HttpClient::fetchUrl("https://www.google.com/");
      if (result.has_value()) {
        successCount++;
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(successCount, 5);
}