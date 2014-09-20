#include "expat_utils.h"

#include <cstring>
#include <iostream>
#include "util.h"

const XML_Char *attr(const XML_Char **atts, const XML_Char *name) {
	for (const XML_Char **a = atts; *a != nullptr; a += 2) {
		if (! ::strcmp(*a, name)) {
			return *(a + 1);
		}
	}

	return NULL;
}

std::string attrString(const XML_Char **atts, const XML_Char *name, const std::string &defaultValue) {
	const XML_Char *attr = ::attr(atts, name);
	if (attr != NULL) {
		return attr;
	} else {
		return defaultValue;
	}
}

uint32_t attrUint(const XML_Char **atts, const XML_Char *name, const uint32_t &defaultValue) {
	const XML_Char *attr = ::attr(atts, name);
	if (attr != NULL) {
		uint32_t outputInt;
		bool conversionOk = ::strToInt(attr, outputInt);

		if (conversionOk) {
			return outputInt;
		} else {
			std::cerr << "Invalid uint value in attribute: " << attr << " (using default: " << defaultValue << ")" << std::endl;
			return defaultValue;
		}
	} else {
		return defaultValue;
	}
}

bool attrBool(const XML_Char **atts, const XML_Char *name, const bool &defaultValue) {
	const XML_Char *attr = ::attr(atts, name);
	if (attr != NULL) {
		bool outputBool;
		bool conversionOk = ::strToBool(attr, outputBool);

		if (conversionOk) {
			return outputBool;
		} else {
			std::cerr << "Invalid bool value in attribute: " << attr << " (using default: " << defaultValue << ")" << std::endl;
			return defaultValue;
		}
	} else {
		return defaultValue;
	}
}
