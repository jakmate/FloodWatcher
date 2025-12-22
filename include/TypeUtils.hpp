#pragma once
#include <simdjson.h>
#include <string>

std::string getString(const simdjson::dom::element& elem, std::string_view key,
                      const char* defaultVal);
int getInt(const simdjson::dom::element& elem, std::string_view key, int defaultVal);
double getDouble(const simdjson::dom::element& elem, std::string_view key, double defaultVal);

template <typename T>
auto getActiveValue(const simdjson::dom::element& elem, std::string_view key, const T& defaultVal) {
  using ValueType = std::remove_cv_t<std::remove_reference_t<decltype(defaultVal)>>;

  simdjson::dom::element field;
  auto error = elem[key].get(field);
  if (error != 0U) {
    return ValueType(defaultVal);
  }

  // Try as array first
  simdjson::dom::array arr;
  if (field.get(arr) == 0U) {
    // It's an array - find active status or use first
    simdjson::dom::array statuses;
    size_t activeIndex = 0;

    if (elem["status"].get(statuses) == 0U && statuses.size() > 0) {
      size_t i = 0;
      for (auto status : statuses) {
        std::string_view statusStr;
        if (!status.get(statusStr) && statusStr.find("statusActive") != std::string_view::npos) {
          activeIndex = i;
          break;
        }
        i++;
      }
    }

    // Get value at activeIndex
    if constexpr (std::is_same_v<ValueType, std::string>) {
      std::string_view sv;
      if (arr.at(activeIndex).get(sv) != 0U) {
        throw std::runtime_error("Array element is not a string");
      }
      return std::string(sv);
    } else if constexpr (std::is_same_v<ValueType, double>) {
      double val;
      if (arr.at(activeIndex).get(val) != 0U) {
        throw std::runtime_error("Array element is not a number");
      }
      return val;
    } else if constexpr (std::is_same_v<ValueType, int64_t>) {
      int64_t val;
      if (arr.at(activeIndex).get(val) != 0U) {
        throw std::runtime_error("Array element is not an integer");
      }
      return val;
    }
  }

  // Not an array - get single value
  if constexpr (std::is_same_v<ValueType, std::string>) {
    std::string_view sv;
    if (field.get(sv) != 0U) {
      throw std::runtime_error("Field is not a string");
    }
    return std::string(sv);
  } else if constexpr (std::is_same_v<ValueType, double>) {
    double val;
    if (field.get(val) != 0U) {
      throw std::runtime_error("Field is not a number");
    }
    return val;
  } else if constexpr (std::is_same_v<ValueType, int64_t>) {
    int64_t val;
    if (field.get(val) != 0U) {
      throw std::runtime_error("Field is not an integer");
    }
    return val;
  }

  return ValueType(defaultVal);
}
