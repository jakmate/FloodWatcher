#include "TypeUtils.hpp"

std::string getString(const simdjson::dom::element& elem, std::string_view key,
                      const char* defaultVal) {
  simdjson::dom::element field;
  auto error = elem[key].get(field);
  if (error != 0U) {
    return defaultVal;
  }

  std::string_view sv;
  if (field.get(sv) != 0U) {
    throw std::runtime_error("Field is not a string");
  }
  return std::string(sv);
}

int getInt(const simdjson::dom::element& elem, std::string_view key, int defaultVal) {
  simdjson::dom::element field;
  auto error = elem[key].get(field);
  if (error != 0U) {
    return defaultVal;
  }

  int64_t val = 0;
  if (field.get(val) != 0U) {
    throw std::runtime_error("Field is not an integer");
  }
  return static_cast<int>(val);
}

double getDouble(const simdjson::dom::element& elem, std::string_view key, double defaultVal) {
  simdjson::dom::element field;
  auto error = elem[key].get(field);
  if (error != 0U) {
    return defaultVal;
  }

  double val = 0.0;
  if (field.get(val) != 0U) {
    throw std::runtime_error("Field is not a number");
  }
  return val;
}
