#pragma once
#include <simdjson.h>
#include <string>

std::string getString(const simdjson::dom::element& elem, std::string_view key,
                      const char* defaultVal);
int getInt(const simdjson::dom::element& elem, std::string_view key, int defaultVal);
double getDouble(const simdjson::dom::element& elem, std::string_view key, double defaultVal);