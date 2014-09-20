#pragma once

#include <zip.h>
#include <iostream>
#include <map>

class Style {

public:
	bool bold = false;

};

class OutlineLevelStyle {

public:
	std::string numFormat;
	bool numLetterSync = false;
	std::string prefix;
	std::string suffix;
	uint32_t startValue = 1;
	uint32_t displayLevels = 1;
	std::string styleName;

};

class Styles {

public:
	using NameToStyle = std::map<std::string, Style>;
	NameToStyle styles;

	using LevelToOutlineLevelStyle = std::map<uint32_t, OutlineLevelStyle>;
	LevelToOutlineLevelStyle outlineLevelStyles;

	const Style &getStyle(const std::string &name) const {
		NameToStyle::const_iterator it = styles.find(name);
		if (it != styles.end()) {
			return it->second;
		} else {
			std::cerr << "Style not found: " << name << std::endl;
			return defaultStyle;
		}
	}

	const OutlineLevelStyle &getOutlineLevelStyle(const uint32_t outlineLevel) const {
		LevelToOutlineLevelStyle::const_iterator it = outlineLevelStyles.find(outlineLevel);
		if (it != outlineLevelStyles.end()) {
			return it->second;
		} else {
			std::cerr << "Outline level style not found for level: " << outlineLevel << std::endl;
			return defaultOutlineLevelStyle;
		}
	}

private:
	Style defaultStyle;
	OutlineLevelStyle defaultOutlineLevelStyle;

};

Styles parseStyles(zip_file *f);
