#include "util.h"

bool strToInt(const std::string &str, uint32_t &outputInt) {
	try {
		size_t pos = 0;
		outputInt = static_cast<uint32_t>(std::stoul(str, &pos));

		return (pos == str.size());
	} catch (...) {
		return false;
	}
}

bool strToBool(const std::string &str, bool &outputBool) {
	if (str == "true") {
		outputBool = true;
		return true;
	}

	if (str == "false") {
		outputBool = false;
		return true;
	}

	return false;
}
