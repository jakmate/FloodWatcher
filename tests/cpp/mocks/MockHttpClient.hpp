// tests/mocks/MockHttpClient.hpp
#pragma once
#include "IHttpClient.hpp"
#include <optional>
#include <string>
#include <unordered_map>

class MockHttpClient : public IHttpClient {
  public:
    std::optional<std::string> fetchUrl(const std::string& url) override {
      auto it = responses_.find(url);
      if (it != responses_.end()) {
        return it->second;
      }
      return std::nullopt;
    }

    void addResponse(const std::string& url, const std::string& response) {
      responses_[url] = response;
    }

  private:
    std::unordered_map<std::string, std::string> responses_;
};