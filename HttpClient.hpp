#pragma once

#include <string>
#include <optional>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
std::optional<std::string> fetchUrl(const std::string& url);