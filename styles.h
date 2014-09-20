#pragma once

#include <zip.h>
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
			return defaultStyle;
		}
	}

	const OutlineLevelStyle &getOutlineLevelStyle(const uint32_t outlineLevel) const {
		LevelToOutlineLevelStyle::const_iterator it = outlineLevelStyles.find(outlineLevel);
		if (it != outlineLevelStyles.end()) {
			return it->second;
		} else {
			return defaultOutlineLevelStyle;
		}
	}

private:
	Style defaultStyle;
	OutlineLevelStyle defaultOutlineLevelStyle;

};

Styles parseStyles(zip_file *f);
