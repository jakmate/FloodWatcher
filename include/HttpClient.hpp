#pragma once
#include <optional>
#include <string>

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
std::optional<std::string> fetchUrl(const std::string& url);
