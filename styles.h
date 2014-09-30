#pragma once

#include <iostream>
#include <map>
#include <boost/optional.hpp>
#include <expat.h>
#include "xmlInZip.h"

class Style {

public:
	std::string parentStyleName;

	boost::optional<bool> bold;
	boost::optional<bool> italic;

	Style apply(const Style &applied) {
		Style newStyle = *this;

		if (applied.bold) {
			newStyle.bold = applied.bold;
		}
		if (applied.italic) {
			newStyle.italic = applied.italic;
		}

		return newStyle;
	}

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

class BulletStyle {

public:

};

class ListStyle {

public:
	using LevelToOutlineLevelStyle = std::map<uint32_t, OutlineLevelStyle>;
	LevelToOutlineLevelStyle outlineLevelStyles;

	using LevelToBulletStyle = std::map<uint32_t, BulletStyle>;
	LevelToBulletStyle bulletStyles;

	bool isNumbered(const uint32_t outlineLevel) const {
		return outlineLevelStyles.find(outlineLevel) != outlineLevelStyles.end();
	}

	const OutlineLevelStyle &getOutlineLevelStyle(const uint32_t outlineLevel) const {
		const LevelToOutlineLevelStyle::const_iterator it = outlineLevelStyles.find(outlineLevel);
		if (it != outlineLevelStyles.end()) {
			return it->second;
		} else {
			std::cerr << "List outline level style not found for level: " << outlineLevel << std::endl;
			return defaultOutlineLevelStyle;
		}
	}

	const BulletStyle &getBulletStyle(const uint32_t outlineLevel) const {
		const LevelToBulletStyle::const_iterator it = bulletStyles.find(outlineLevel);
		if (it != bulletStyles.end()) {
			return it->second;
		} else {
			std::cerr << "List bullet style not found for level: " << outlineLevel << std::endl;
			return defaultBulletStyle;
		}
	}

private:
	OutlineLevelStyle defaultOutlineLevelStyle;
	BulletStyle defaultBulletStyle;

};

class Styles {

public:
	using NameToStyle = std::map<std::string, Style>;
	NameToStyle styles;

	using LevelToOutlineLevelStyle = std::map<uint32_t, OutlineLevelStyle>;
	LevelToOutlineLevelStyle outlineLevelStyles;

	using NameToListStyle = std::map<std::string, ListStyle>;
	NameToListStyle listStyles;

	Style getMergedStyle(const std::string &name) const {
		const Style &style = getStyle(name);
		if (! style.parentStyleName.empty()) {
			Style mergedStyle = getMergedStyle(style.parentStyleName);

			return mergedStyle.apply(style);
		} else {
			return style;
		}
	}

	const OutlineLevelStyle &getOutlineLevelStyle(const uint32_t outlineLevel) const {
		const LevelToOutlineLevelStyle::const_iterator it = outlineLevelStyles.find(outlineLevel);
		if (it != outlineLevelStyles.end()) {
			return it->second;
		} else {
			std::cerr << "Outline level style not found for level: " << outlineLevel << std::endl;
			return defaultOutlineLevelStyle;
		}
	}

	const ListStyle &getListStyle(const std::string &name) const {
		const NameToListStyle::const_iterator it = listStyles.find(name);
		if (it != listStyles.end()) {
			return it->second;
		} else {
			std::cerr << "List style not found: " << name << std::endl;
			return defaultListStyle;
		}
	}

private:
	const Style &getStyle(const std::string &name) const {
		NameToStyle::const_iterator it = styles.find(name);
		if (it != styles.end()) {
			return it->second;
		} else {
			std::cerr << "Style not found: " << name << std::endl;
			return defaultStyle;
		}
	}

private:
	Style defaultStyle;
	OutlineLevelStyle defaultOutlineLevelStyle;
	ListStyle defaultListStyle;

};

class StylesContext {

public:
	Styles styles;

	Style *currentStyle = nullptr;
	ListStyle *currentListStyle = nullptr;

};

class StylesHandler : public ExpatXmlHandler {
public:
	StylesHandler(StylesContext &context);

public:
	void onStart(const XML_Char *name, const XML_Char **atts) override;
	void onEnd(const XML_Char *name) override;

private:
	StylesContext &context;
};
