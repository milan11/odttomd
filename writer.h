#pragma once

#include <iosfwd>
#include <cstdint>

class Writer {

public:
	Writer(std::ostream &os);

	void write(const char c);
	void write(const std::string &str);

	void resetCodePointsCount();
	uint32_t getCodePointsCount() const;

private:
	std::ostream &os;
	uint32_t codePointsCount = 0;

};
