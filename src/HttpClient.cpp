#include "HttpClient.hpp"
#include <curl/curl.h>
#include <iostream>

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
  userp->append(static_cast<char*>(contents), size * nmemb);
  return size * nmemb;
}

// Helper function to fetch URL content
std::optional<std::string> fetchUrl(const std::string& url) {
  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    std::cerr << "Failed to initialize curl for URL: " << url << '\n';
    return std::nullopt;
  }

  std::string readBuffer;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    std::cerr << "CURL error: " << curl_easy_strerror(res) << '\n';
    return std::nullopt;
  }

  if (httpCode != 200) {
    std::cerr << "HTTP Error " << httpCode << ":\n" << readBuffer << '\n';
    return std::nullopt;
  }

  return readBuffer;
}
