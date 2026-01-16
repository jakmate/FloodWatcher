#pragma once
#include <optional>
#include <string>
#include <vector>

class IHttpClient {
  public:
    IHttpClient() = default;
    virtual ~IHttpClient() = default;

    IHttpClient(const IHttpClient&) = delete;
    IHttpClient& operator=(const IHttpClient&) = delete;
    IHttpClient(IHttpClient&&) = delete;
    IHttpClient& operator=(IHttpClient&&) = delete;

    virtual std::optional<std::string> fetchUrl(const std::string& url) = 0;
    virtual std::vector<std::optional<std::string>>
    fetchUrls(const std::vector<std::string>& urls) = 0;
};
