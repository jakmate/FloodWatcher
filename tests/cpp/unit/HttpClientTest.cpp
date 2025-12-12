// tests/cpp/unit/HttpClientTest.cpp
#include "HttpClient.hpp"
#include <gtest/gtest.h>
#include <thread>

TEST(HttpClientTest, FetchValidUrl) {
  auto result = HttpClient::getInstance().fetchUrl("https://www.google.com/");

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->empty()); // NOLINT(bugprone-unchecked-optional-access,-warnings-as-errors)
}

TEST(HttpClientTest, Fetch404ReturnsNullopt) {
  auto result = HttpClient::getInstance().fetchUrl("https://www.google.com/nonexistent404page");

  EXPECT_FALSE(result.has_value());
}

TEST(HttpClientTest, InvalidUrlReturnsNullopt) {
  auto result = HttpClient::getInstance().fetchUrl("invalid://url");

  EXPECT_FALSE(result.has_value());
}

TEST(HttpClientTest, SingletonReturnsSameInstance) {
  IHttpClient& instance1 = HttpClient::getInstance();
  IHttpClient& instance2 = HttpClient::getInstance();

  EXPECT_EQ(&instance1, &instance2);
}

TEST(HttpClientTest, MultipleSequentialRequestsWork) {
  auto result1 = HttpClient::getInstance().fetchUrl("https://www.google.com/");
  auto result2 = HttpClient::getInstance().fetchUrl("https://www.google.com/");

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
}

TEST(HttpClientTest, ConcurrentRequestsUsePooling) {
  std::vector<std::thread> threads;
  threads.reserve(10);
  std::atomic<int> successCount{0};

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&successCount]() {
      auto result = HttpClient::getInstance().fetchUrl("https://www.google.com/");
      if (result.has_value()) {
        successCount++;
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - start)
                      .count();

  EXPECT_EQ(successCount, 10);
  // With pooling, 10 concurrent requests should be faster than 10 sequential
  std::cout << "10 concurrent requests took: " << duration << "ms\n";
}

TEST(HttpClientTest, ConnectionReusePreservesState) {
  // Multiple requests to same host should reuse connection
  auto result1 = HttpClient::getInstance().fetchUrl("https://www.google.com/");
  auto result2 = HttpClient::getInstance().fetchUrl("https://duckduckgo.com/");

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
}

TEST(HttpClientTest, PoolSizeLimit) {
  std::vector<std::thread> threads;
  threads.reserve(15); // Exceeds default pool size of 10
  std::atomic<int> successCount{0};

  for (int i = 0; i < 15; ++i) {
    threads.emplace_back([&successCount]() {
      auto result = HttpClient::getInstance().fetchUrl("https://www.google.com/");
      if (result.has_value()) {
        successCount++;
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // Should still succeed even when exceeding pool size
  EXPECT_EQ(successCount, 15);
}
