#pragma once

#include <cstdint>
#include <string>

class Writer {

public:
	virtual ~Writer() {}

	virtual void writeMarkup(const char c) = 0;
	virtual void writeMarkup(const std::string &str) = 0;

	virtual void writeVisibleText(const char c) = 0;
	virtual void writeVisibleText(const std::string &str) = 0;

	virtual void writeVisibleText_escaped(const char c) = 0;
	virtual void writeVisibleText_escaped(const std::string &str) = 0;

};
