#pragma once

#include <string>
#include <vector>

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

void initProfiles();
std::vector<std::string> getAvailableProfiles();
void setProfile(const std::string &name);
const Options &options();
