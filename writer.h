#pragma once

#include <iosfwd>
#include <cstdint>

class Writer {

public:
	Writer(std::ostream &os);

	void writeMarkup(const char c);
	void writeMarkup(const std::string &str);

	void writeVisibleText(const char c);
	void writeVisibleText(const std::string &str);

	void writeVisibleText_escaped(const char c);
	void writeVisibleText_escaped(const std::string &str);

	void resetCodePointsCount();
	uint32_t getCodePointsCount() const;

private:
	void write(const char c);
	void write(const std::string &str);

private:
	std::ostream &os;
	uint32_t codePointsCount = 0;

};
