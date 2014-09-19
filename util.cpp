#include "util.h"

uint32_t strToInt(const std::string &str, const uint32_t defaultInt, const bool noErrorIfEmpty) {
	try {
		size_t pos = 0;
		uint32_t result = static_cast<uint32_t>(std::stoul(str, &pos));

		if (pos == str.size()) {
			return result;
		} else {
			throw 0;
		}
	} catch (...) {
		if ((! noErrorIfEmpty) || (! str.empty())) {
			throw "invalid number: (" + str + ")";
		}
		return defaultInt;
	}
}

bool strToBool(const std::string &str, const bool defaultBool, const bool noErrorIfEmpty) {
	if (str == "true") return true;
	if (str == "false") return false;

	if ((! noErrorIfEmpty) || (! str.empty())) {
		throw "invalid bool: (" + str + ")";
	}

	return defaultBool;
}
