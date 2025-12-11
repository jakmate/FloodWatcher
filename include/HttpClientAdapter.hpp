#pragma once
#include "HttpClient.hpp"
#include "IHttpClient.hpp"
#include <optional>
#include <string>

class HttpClientAdapter : public IHttpClient {
  public:
    std::optional<std::string> fetchUrl(const std::string& url) override {
      return HttpClient::getInstance().fetchUrl(url);
    }
};
