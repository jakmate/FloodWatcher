#include "HttpClient.hpp"
#include <iostream>
#include <thread>

// Static members initialization
bool HttpClient::curlInitialized = false;
std::once_flag HttpClient::curlInitFlag;
IHttpClient* HttpClient::testInstance = nullptr;

HttpClient::HttpClient(size_t poolSize) : poolSize_(poolSize), createdHandles_(0) {
  std::call_once(curlInitFlag, []() {
    curl_global_init(CURL_GLOBAL_ALL);
    curlInitialized = true;
    std::atexit(cleanupCurl);
  });
}

HttpClient::~HttpClient() {
  std::scoped_lock<std::mutex> lock(poolMutex_);
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
  curl_easy_setopt(curl, CURLOPT_MAXCONNECTS, 5L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
  #ifdef _WIN32
  // Use Windows native certificate store
  curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
  #endif
}

HttpClient::CurlHandle HttpClient::acquireHandle() {
  std::scoped_lock<std::mutex> lock(poolMutex_);

  // Try to get from pool first
  if (!availableHandles_.empty()) {
    CURL* handle = availableHandles_.front();
    availableHandles_.pop_front();
    return {handle, this};
    ;
  }

  // Create new handle if under pool limit
  if (createdHandles_ < poolSize_) {
    CURL* handle = curl_easy_init();
    if (handle == nullptr) {
      std::cerr << "Failed to initialize curl handle\n";
      return {nullptr, this};
    }
    applyPersistentOptions(handle);
    createdHandles_++;
    return {handle, this};
  }

  // Pool exhausted, wait for a handle to be returned
  // For now, create a temporary handle (could add waiting logic)
  CURL* handle = curl_easy_init();
  if (handle != nullptr) {
    applyPersistentOptions(handle);
  }
  return {handle, nullptr}; // nullptr client = won't return to pool
}

void HttpClient::returnHandle(CURL* handle) {
  if (handle == nullptr) {
    return;
  }

  std::scoped_lock<std::mutex> lock(poolMutex_);

  // Return to pool if not over capacity
  if (availableHandles_.size() < poolSize_) {
    availableHandles_.push_back(handle);
  } else {
    curl_easy_cleanup(handle);
  }
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
