#include "writer_collecting.h"

#include <iostream>
#include "options.h"

void Writer_Collecting::resetCollectedVisibleText()
{
	collectedVisibleText.str("");
}

std::string Writer_Collecting::getCollectedVisibleText() const
{
	return collectedVisibleText.str();
}

void Writer_Collecting::writeMarkup(const char)
{
}

void Writer_Collecting::writeMarkup(const std::string &)
{
}

void Writer_Collecting::writeVisibleText(const char c)
{
	write(c);
}

void Writer_Collecting::writeVisibleText(const std::string &str)
{
	write(str);
}

void Writer_Collecting::writeVisibleText_escaped(const char c)
{
	write(c);
}

void Writer_Collecting::writeVisibleText_escaped(const std::string &str)
{
	write(str);
}

void Writer_Collecting::write(const char c) {
	collectedVisibleText << c;
}

void Writer_Collecting::write(const std::string &str) {
	for (const char c : str) {
		write(c);
	}
}
