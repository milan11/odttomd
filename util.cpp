#include "util.h"

uint32_t strToInt(const std::string &str, const uint32_t defaultInt) {
	try {
		size_t pos = 0;
		uint32_t result = static_cast<uint32_t>(std::stoul(str, &pos));

		if (pos == str.size()) {
			return result;
		} else {
			throw 0;
		}
	} catch (...) {
		return defaultInt;
	}
}
