#include "HttpClient.hpp"
#include <iostream>
#include <thread>

// Static members initialization
bool HttpClient::curlInitialized = false;
std::once_flag HttpClient::curlInitFlag;
IHttpClient* HttpClient::testInstance = nullptr;

HttpClient::HttpClient(size_t poolSize)
    : poolSize_(poolSize), createdHandles_(0), inUseHandles_(0) {
  std::call_once(curlInitFlag, []() {
    curl_global_init(CURL_GLOBAL_ALL);
    curlInitialized = true;
    std::atexit(cleanupCurl);
  });
}

HttpClient::~HttpClient() {
  std::unique_lock<std::mutex> lock(poolMutex_);

  // Wait for all handles to be returned
  poolCond_.wait(lock, [this]() { return inUseHandles_ == 0; });

  for (CURL* handle : availableHandles_) {
    curl_easy_cleanup(handle);
  }
  availableHandles_.clear();
}

void HttpClient::cleanupCurl() {
  if (curlInitialized) {
    curl_global_cleanup();
    curlInitialized = false;
  }
}

IHttpClient& HttpClient::getInstance() {
  if (testInstance != nullptr) {
    return *testInstance;
  }
  static HttpClient instance;
  return instance;
}

void HttpClient::setInstance(IHttpClient* instance) {
  testInstance = instance;
}

void HttpClient::applyPersistentOptions(CURL* curl) {
  // Set common options that persist across requests
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);        // 30 second timeout
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); // 10 second connect timeout
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);   // Enable keep-alive
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // Enable gzip/deflate
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
}

HttpClient::CurlHandle HttpClient::acquireHandle() {
  std::unique_lock<std::mutex> lock(poolMutex_);

  // Wait until either there's an available handle in the pool, OR
  // We can create a new handle under pool limit
  poolCond_.wait(lock,
                 [this]() { return !availableHandles_.empty() || createdHandles_ < poolSize_; });

  // If there's an available handle, use it
  CURL* handle = nullptr;
  if (!availableHandles_.empty()) {
    handle = availableHandles_.front();
    availableHandles_.pop_front();
  } else {
    // Otherwise, create a new handle
    handle = curl_easy_init();
    if (handle == nullptr) {
      std::cerr << "Failed to initialize curl handle\n";
      return {nullptr, this};
    }
    applyPersistentOptions(handle);
    createdHandles_++;
  }
  inUseHandles_++;
  return {handle, this};
}

void HttpClient::returnHandle(CURL* handle) {
  if (handle == nullptr) {
    return;
  }

  std::unique_lock<std::mutex> lock(poolMutex_);
  availableHandles_.push_back(handle); // Return to pool
  inUseHandles_--;                     // Decrement in-use count
  poolCond_.notify_one();              // Notify one waiting thread that a handle is available
}

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
  userp->append(static_cast<char*>(contents), size * nmemb);
  return size * nmemb;
}

std::optional<std::string> HttpClient::fetchUrl(const std::string& url) {
  CurlHandle curlWrapper = acquireHandle();
  CURL* curl = curlWrapper.get();

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
