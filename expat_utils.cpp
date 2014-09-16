#include "expat_utils.h"

#include <cstring>

const XML_Char *attr(const XML_Char **atts, const XML_Char *name) {
	for (const XML_Char **a = atts; *a != nullptr; a += 2) {
		if (! ::strcmp(*a, name)) {
			return *(a + 1);
		}
	}

	return "";
}
