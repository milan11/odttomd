#include "writer.h"

#include <iostream>

Writer::Writer(std::ostream &os)
	: os(os)
{

}

void Writer::write(const char c) {
	os << c;

	if (((c & 0x80) == 0) || ((c & 0xc0) == 0xc0)) {
		++codePointsCount;
	}
}

void Writer::write(const std::string &str) {
	for (const char c : str) {
		write(c);
	}
}

void Writer::resetCodePointsCount() {
	codePointsCount = 0;
}

uint32_t Writer::getCodePointsCount() const {
	return codePointsCount;
}

