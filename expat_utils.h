#pragma once

#include <cstdint>
#include <string>
#include <expat.h>

std::string attrString(const XML_Char **atts, const XML_Char *name, const std::string &defaultValue);
uint32_t attrUint(const XML_Char **atts, const XML_Char *name, const uint32_t &defaultValue);
bool attrBool(const XML_Char **atts, const XML_Char *name, const bool &defaultValue);
