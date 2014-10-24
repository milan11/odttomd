#include "writer.h"

#include <iostream>
#include "options.h"

Writer::Writer(std::ostream &os)
	: os(os)
{

}

void Writer::writeMarkup(const char c)
{
	write(c);
}

void Writer::writeMarkup(const std::string &str)
{
	write(str);
}

void Writer::writeVisibleText(const char c)
{
	write(c);
}

void Writer::writeVisibleText(const std::string &str)
{
	write(str);
}

void Writer::writeVisibleText_escaped(const char c)
{
	if (options().escapeInText.find(c) != std::string::npos) {
		write('\\');
		write(c);
	}
	else if (options().entitiesInText.find(c) != std::string::npos) {
		switch (c) {
			case '<':
				write("&lt;");
				break;
			case '>':
				write("&gt;");
				break;
			case '"':
				write("&quot;");
				break;
			case '&':
				write("&amp;");
				break;
			case '\'':
				write("&apos;");
				break;
			default:
				write(c);
		}
	}
	else {
		write(c);
	}
}

void Writer::writeVisibleText_escaped(const std::string &str)
{
	for (const char c : str) {
		writeVisibleText_escaped(c);
	}
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

