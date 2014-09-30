#include "options.h"

#include <iostream>
#include <map>

using Profiles = std::vector<std::pair<std::string, Options> >;
Profiles profiles;

Options currentOptions;

void initProfiles() {
	{
		profiles.push_back(std::make_pair("standard", Options()));
		Options &options = profiles.back().second;
		options.headingNumberFormats = true;
		options.listNumberFormats = false;
		options.escapeDotInHeadingNumbers = false;
		options.escapeDotInListNumbers = false;
		options.escapeInText = "\\`*_{}[]()#+-!";
		options.entitiesInText = "<>";
		options.bold = true;
		options.italic = true;
		options.stylesInHeadingNumbers = true;
		options.stylesInListNumbers = false;
		options.headingNumbersStartValue = true;
		options.listNumbersStartValue = false;
		options.headingNumbersLevels = true;
		options.listNumbersLevels = false;
		options.edgeSpacesInsideBoldItalic = true;
		options.boldItalicBeginInsideOfWords = true;
		options.boldItalicEndInsideOfWords = true;
		options.linksToHeadings = false;
		options.underlineHeading1 = true;
		options.underlineHeading2 = true;
		options.closeHeadings = false;
		options.closeHeadingsShort = false;
	}
	{
		profiles.push_back(std::make_pair("github", Options()));
		Options &options = profiles.back().second;
		options.headingNumberFormats = true;
		options.listNumberFormats = false;
		options.escapeDotInHeadingNumbers = false;
		options.escapeDotInListNumbers = false;
		options.escapeInText = "\\`*_{}[]()#+-!";
		options.entitiesInText = "<>";
		options.bold = true;
		options.italic = true;
		options.stylesInHeadingNumbers = true;
		options.stylesInListNumbers = false;
		options.headingNumbersStartValue = true;
		options.listNumbersStartValue = false;
		options.headingNumbersLevels = true;
		options.listNumbersLevels = false;
		options.edgeSpacesInsideBoldItalic = false;
		options.boldItalicBeginInsideOfWords = false;
		options.boldItalicEndInsideOfWords = true;
		options.linksToHeadings = true;
		options.underlineHeading1 = true;
		options.underlineHeading2 = true;
		options.closeHeadings = false;
		options.closeHeadingsShort = false;
	}
	{
		profiles.push_back(std::make_pair("plain_text", Options()));
		Options &options = profiles.back().second;
		options.headingNumberFormats = true;
		options.listNumberFormats = true;
		options.escapeDotInHeadingNumbers = false;
		options.escapeDotInListNumbers = false;
		options.escapeInText = "";
		options.entitiesInText = "";
		options.bold = false;
		options.italic = false;
		options.stylesInHeadingNumbers = false;
		options.stylesInListNumbers = false;
		options.headingNumbersStartValue = true;
		options.listNumbersStartValue = true;
		options.headingNumbersLevels = true;
		options.listNumbersLevels = true;
		options.edgeSpacesInsideBoldItalic = true;
		options.boldItalicBeginInsideOfWords = true;
		options.boldItalicEndInsideOfWords = true;
		options.linksToHeadings = false;
		options.underlineHeading1 = true;
		options.underlineHeading2 = true;
		options.closeHeadings = true;
		options.closeHeadingsShort = false;
	}
	{
		profiles.push_back(std::make_pair("readable", Options()));
		Options &options = profiles.back().second;
		options.headingNumberFormats = true;
		options.listNumberFormats = false;
		options.escapeDotInHeadingNumbers = false;
		options.escapeDotInListNumbers = false;
		options.escapeInText = "\\`*_{}[]()#+-!";
		options.entitiesInText = "";
		options.bold = true;
		options.italic = true;
		options.stylesInHeadingNumbers = false;
		options.stylesInListNumbers = false;
		options.headingNumbersStartValue = true;
		options.listNumbersStartValue = false;
		options.headingNumbersLevels = true;
		options.listNumbersLevels = false;
		options.edgeSpacesInsideBoldItalic = true;
		options.boldItalicBeginInsideOfWords = true;
		options.boldItalicEndInsideOfWords = true;
		options.linksToHeadings = false;
		options.underlineHeading1 = true;
		options.underlineHeading2 = true;
		options.closeHeadings = true;
		options.closeHeadingsShort = false;
	}
	{
		profiles.push_back(std::make_pair("parseable", Options()));
		Options &options = profiles.back().second;
		options.headingNumberFormats = false;
		options.listNumberFormats = false;
		options.escapeDotInHeadingNumbers = false;
		options.escapeDotInListNumbers = false;
		options.escapeInText = "\\`*_{}[]()#+-!.<>";
		options.bold = true;
		options.italic = true;
		options.stylesInHeadingNumbers = false;
		options.stylesInListNumbers = false;
		options.headingNumbersStartValue = false;
		options.listNumbersStartValue = false;
		options.headingNumbersLevels = false;
		options.listNumbersLevels = false;
		options.edgeSpacesInsideBoldItalic = true;
		options.boldItalicBeginInsideOfWords = true;
		options.boldItalicEndInsideOfWords = true;
		options.linksToHeadings = false;
		options.underlineHeading1 = false;
		options.underlineHeading2 = false;
		options.closeHeadings = false;
		options.closeHeadingsShort = false;
	}
}

std::vector<std::string> getAvailableProfiles() {
	std::vector<std::string> result;
	result.reserve(profiles.size());
	for (auto &kv : profiles) {
		result.push_back(kv.first);
	}
	return result;
}

void setProfile(const std::string &name) {
	for (auto &kv : profiles) {
		if (kv.first == name) {
			currentOptions = kv.second;
			return;
		}
	}
	throw "Unknown profile: " + name;
}

const Options &options() {
	return currentOptions;
}
