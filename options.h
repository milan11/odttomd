#pragma once

#include <string>

struct Options {
	//bool supportHeadingsNumberFormats = false;
	//bool supportListNumberFormats = false;
	//bool escapeDotInHeadingNumbers = false;
	//bool escapeDotInListNumbers = false;
	//bool escapeDotInText = false;
	bool supportBold = false;
	bool supportItalic = false;
	//bool supportStylesInHeadingNumbers = false;
	//bool supportStylesInListNumbers = false;
};

void setProfile(const std::string &name);
Options &options();
