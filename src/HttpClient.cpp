#include "HttpClient.hpp"
#include <iostream>
#include <thread>

// Static members initialization
bool HttpClient::curlInitialized = false;
std::once_flag HttpClient::curlInitFlag;
thread_local CURL* HttpClient::threadCurlHandle = nullptr;

HttpClient::HttpClient() {
  std::call_once(curlInitFlag, []() {
    curl_global_init(CURL_GLOBAL_ALL);
    curlInitialized = true;
    std::atexit(cleanupCurl);
  });
}

HttpClient::~HttpClient() {
  // Thread-specific cleanup
  if (threadCurlHandle != nullptr) {
    curl_easy_cleanup(threadCurlHandle);
    threadCurlHandle = nullptr;
  }
}

void HttpClient::cleanupCurl() {
  if (curlInitialized) {
    curl_global_cleanup();
    curlInitialized = false;
  }
}

HttpClient& HttpClient::getInstance() {
  static HttpClient instance;
  return instance;
}

CURL* HttpClient::getThreadCurlHandle() {
  if (threadCurlHandle == nullptr) {
    threadCurlHandle = curl_easy_init();
    if (threadCurlHandle == nullptr) {
      std::cerr << "Failed to initialize curl handle for thread " << std::this_thread::get_id()
                << '\n';
      return nullptr;
    }

    // Set common options that persist across requests
    curl_easy_setopt(threadCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(threadCurlHandle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(threadCurlHandle, CURLOPT_TIMEOUT, 30L);        // 30 second timeout
    curl_easy_setopt(threadCurlHandle, CURLOPT_CONNECTTIMEOUT, 10L); // 10 second connect timeout
    curl_easy_setopt(threadCurlHandle, CURLOPT_TCP_KEEPALIVE, 1L);   // Enable keep-alive
    curl_easy_setopt(threadCurlHandle, CURLOPT_ACCEPT_ENCODING, ""); // Enable gzip/deflate
  }
  return threadCurlHandle;
}

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
  userp->append(static_cast<char*>(contents), size * nmemb);
  return size * nmemb;
}

std::optional<std::string> HttpClient::fetchUrl(const std::string& url) {
  CURL* curl = getThreadCurlHandle();
  if (curl == nullptr) {
    std::cerr << "No curl handle available for URL: " << url << '\n';
    return std::nullopt;
  }

  std::string readBuffer;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

  CURLcode res = curl_easy_perform(curl);
  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  // Reset the handle for next use (keeps connection alive)
  curl_easy_reset(curl);

  // Re-apply persistent settings
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

  if (res != CURLE_OK) {
    std::cerr << "CURL error for " << url << ": " << curl_easy_strerror(res) << '\n';
    return std::nullopt;
  }

  if (httpCode != 200) {
    std::cerr << "HTTP Error " << httpCode << " for URL: " << url << '\n';
    // std::cerr << "Response: " << readBuffer << '\n';
    return std::nullopt;
  }

  return readBuffer;
}
