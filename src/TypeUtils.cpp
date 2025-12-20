#include "TypeUtils.hpp"

std::string getString(const simdjson::dom::element& elem, std::string_view key,
                      const char* defaultVal) {
  std::string_view sv;
  if (elem[key].get(sv) == 0U) {
    return std::string(sv);
  }
  return defaultVal;
}

int getInt(const simdjson::dom::element& elem, std::string_view key, int defaultVal) {
  int64_t val = 0;
  if (elem[key].get(val) == 0U) {
    return static_cast<int>(val);
  }
  return defaultVal;
}

double getDouble(const simdjson::dom::element& elem, std::string_view key, double defaultVal) {
  double val = 0.0;
  if (elem[key].get(val) == 0U) {
    return val;
  }
  return defaultVal;
}