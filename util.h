#pragma once

#include <string>

uint32_t strToInt(const std::string &str, const uint32_t defaultInt, const bool noErrorIfEmpty);
bool strToBool(const std::string &str, const bool defaultBool, const bool noErrorIfEmpty);
