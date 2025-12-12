#pragma once
#include <curl/curl.h>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include "IHttpClient.hpp"

class HttpClient : public IHttpClient {
  public:
    HttpClient(size_t poolSize = 10);
    ~HttpClient();
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) = delete;
    HttpClient& operator=(HttpClient&&) = delete;

    // Fetch URL using connection reuse
    std::optional<std::string> fetchUrl(const std::string& url) override;

    // Get singleton instance
    static HttpClient& getInstance();
    static void setInstance(IHttpClient* instance);

  private:
    // RAII wrapper for CURL handle with automatic return to pool
    class CurlHandle {
      public:
        CurlHandle(CURL* handle, HttpClient* client) : handle_(handle), client_(client) {}
        ~CurlHandle() {
          if (handle_ && client_) {
            client_->returnHandle(handle_);
          }
        }

        CurlHandle(const CurlHandle&) = delete;
        CurlHandle& operator=(const CurlHandle&) = delete;
        CurlHandle(CurlHandle&& other) noexcept : handle_(other.handle_), client_(other.client_) {
          other.handle_ = nullptr;
          other.client_ = nullptr;
        }

        CURL* get() const {
          return handle_;
        }

      private:
        CURL* handle_;
        HttpClient* client_;
    };

    // Pool management
    std::deque<CURL*> availableHandles_;
    std::mutex poolMutex_;
    size_t poolSize_;
    size_t createdHandles_;

    // Get handle from pool
    CurlHandle acquireHandle();
    void returnHandle(CURL* handle);

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
    static IHttpClient* testInstance;
};
