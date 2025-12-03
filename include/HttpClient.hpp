#pragma once
#include <curl/curl.h>
#include <mutex>
#include <optional>
#include <string>

class HttpClient {
  public:
    HttpClient();
    ~HttpClient();
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) = delete;
    HttpClient& operator=(HttpClient&&) = delete;

    // Fetch URL using connection reuse
    static std::optional<std::string> fetchUrl(const std::string& url);

    // Get singleton instance
    static HttpClient& getInstance();

  private:
    // Thread-local storage for curl handles to avoid thread safety issues
    static thread_local CURL* threadCurlHandle;

    // Initialize curl handle for current thread
    static CURL* getThreadCurlHandle();

    // Set common options that persist across requests
    static void applyPersistentOptions(CURL* curl);

    // Write callback as member function
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

    // Global curl initialization
    static void initCurl();
    static void cleanupCurl();

    // Track initialization state
    static bool curlInitialized;
    static std::once_flag curlInitFlag;
};
