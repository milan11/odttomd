#pragma once

#include <string>

struct Options {
	bool headingNumberFormats = false;
	bool listNumberFormats = false;
	bool escapeDotInHeadingNumbers = false;
	bool escapeDotInListNumbers = false;
	bool escapeDotInText = false;
	bool bold = false;
	bool italic = false;
	bool stylesInHeadingNumbers = false;
	bool stylesInListNumbers = false;
};

void setProfile(const std::string &name);
Options &options();
