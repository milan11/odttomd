#pragma once

#include <string>
#include <vector>

struct Options {
	bool headingNumberFormats = false;
	bool listNumberFormats = false;
	bool escapeDotInHeadingNumbers = false;
	bool escapeDotInListNumbers = false;
	std::string escapeInText;
	std::string entitiesInText;
	bool bold = false;
	bool italic = false;
	bool stylesInHeadingNumbers = false;
	bool stylesInListNumbers = false;
	bool headingNumbersStartValue = false;
	bool listNumbersStartValue = false;
	bool headingNumbersLevels = false;
	bool listNumbersLevels = false;
	bool edgeSpacesInsideBoldItalic = false;
	bool boldItalicBeginInsideOfWords = false;
	bool boldItalicEndInsideOfWords = false;
	bool linksToHeadings = false;
	bool underlineHeading1 = false;
	bool underlineHeading2 = false;
	bool closeHeadings = false;
	bool closeHeadingsShort = false;
};

void initProfiles();
std::vector<std::string> getAvailableProfiles();
void setProfile(const std::string &name);
const Options &options();
