#pragma once
#include <optional>
#include <string>

class IHttpClient {
  public:
    IHttpClient() = default;
    virtual ~IHttpClient() = default;

    IHttpClient(const IHttpClient&) = delete;
    IHttpClient& operator=(const IHttpClient&) = delete;
    IHttpClient(IHttpClient&&) = delete;
    IHttpClient& operator=(IHttpClient&&) = delete;

    virtual std::optional<std::string> fetchUrl(const std::string& url) = 0;
};
