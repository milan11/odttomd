#include "writer_output.h"

#include <iostream>
#include "options.h"

Writer_Output::Writer_Output(std::ostream &os)
	: os(os)
{
}

void Writer_Output::resetCodePointsCount() {
	codePointsCount = 0;
}

uint32_t Writer_Output::getCodePointsCount() const {
	return codePointsCount;
}

void Writer_Output::writeMarkup(const char c)
{
	write(c);
}

void Writer_Output::writeMarkup(const std::string &str)
{
	write(str);
}

void Writer_Output::writeVisibleText(const char c)
{
	write(c);
}

void Writer_Output::writeVisibleText(const std::string &str)
{
	write(str);
}

void Writer_Output::writeVisibleText_escaped(const char c)
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

void Writer_Output::writeVisibleText_escaped(const std::string &str)
{
	for (const char c : str) {
		writeVisibleText_escaped(c);
	}
}

void Writer_Output::write(const char c) {
	os << c;

	if (((c & 0x80) == 0) || ((c & 0xc0) == 0xc0)) {
		++codePointsCount;
	}
}

void Writer_Output::write(const std::string &str) {
	for (const char c : str) {
		write(c);
	}
}
