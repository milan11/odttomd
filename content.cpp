#include "content.h"

#include <algorithm>
#include <iostream>
#include <stack>
#include <vector>
#include <boost/scope_exit.hpp>
#include <expat.h>
#include "expat_utils.h"
#include "numbering.h"
#include "options.h"
#include "util.h"

namespace {

class List {

public:
	std::string listStyleName;
	std::vector<uint32_t> currentNumbering;
};

class Context {

public:
	Context(StylesContext &stylesContext)
		: stylesContext(stylesContext)
	{
		currentStyles.push(Style());
	}

public:
	StylesContext stylesContext;

	std::stack<Style> currentStyles;
	Style currentStyle;
	bool outerIsItalic = false;

	std::string currentUrl;
	std::vector<uint32_t> currentOutlineNumbering;

	std::stack<List> currentLists;

	uint32_t remainingSpacesCount = 0;
	bool lastWasWhitespace = true;
	bool lastWasStyleEnd = false;

};

void writeRemainingSpaces(Context &context) {
	std::cout << std::string(context.remainingSpacesCount, ' ');

	if (context.remainingSpacesCount > 0) {
		context.lastWasStyleEnd = false;
	}

	context.remainingSpacesCount = 0;
}

void writeSpaceBeforeStyleIfNeeded(Context &context) {
	if ((! options().boldItalicBeginInsideOfWords) && (! context.lastWasWhitespace)) {
		std::cout << ' ';
		context.lastWasWhitespace = true;
	}
}

void writeSpaceAfterStyleIfNeeded(Context &context, const char currentChar) {
	if ((! options().boldItalicEndInsideOfWords) && (context.lastWasStyleEnd)) {
		if (currentChar != ' ' && currentChar != '\n')
			std::cout << ' ';
	}

	context.lastWasStyleEnd = false;
}

void writeStyleDiff(Context &context, const Style &oldStyle, const Style &newStyle) {
	const bool enablingBold = options().bold && (!oldStyle.bold.get_value_or(false) && newStyle.bold.get_value_or(false));
	bool disablingBold = options().bold && (oldStyle.bold.get_value_or(false) && !newStyle.bold.get_value_or(false));
	const bool enablingItalic = options().italic && (!oldStyle.italic.get_value_or(false) && newStyle.italic.get_value_or(false));
	const bool disablingItalic = options().italic && (oldStyle.italic.get_value_or(false) && !newStyle.italic.get_value_or(false));

	const bool inItalic = oldStyle.italic.get_value_or(false) && (! disablingItalic);
	const bool inBold = oldStyle.bold.get_value_or(false) && (! disablingBold);

	if (disablingItalic) {
		if ((context.outerIsItalic) && (inBold || disablingBold)) {
			std::cout << "**";
			disablingBold = false;
		}

		std::cout << "_";
		context.lastWasStyleEnd = true;

		if ((context.outerIsItalic) && (inBold)) {
			::writeRemainingSpaces(context);
			::writeSpaceBeforeStyleIfNeeded(context);

			std::cout << "**";
			context.lastWasStyleEnd = false;

			context.outerIsItalic = false;
		}
	}

	if (disablingBold) {
		if ((! context.outerIsItalic) && (inItalic)) {
			std::cout << "_";
		}

		std::cout << "**";
		context.lastWasStyleEnd = true;

		if ((! context.outerIsItalic) && (inItalic)) {
			::writeRemainingSpaces(context);
			::writeSpaceBeforeStyleIfNeeded(context);

			std::cout << "_";
			context.lastWasStyleEnd = false;

			context.outerIsItalic = true;
		}
	}

	if (enablingBold) {
		if ((! inItalic) || enablingItalic)
			context.outerIsItalic = false;

		::writeRemainingSpaces(context);
		::writeSpaceBeforeStyleIfNeeded(context);

		std::cout << "**";
		context.lastWasStyleEnd = false;
	}

	if (enablingItalic) {
		if ((! inBold) && (! enablingBold))
			context.outerIsItalic = true;

		::writeRemainingSpaces(context);
		::writeSpaceBeforeStyleIfNeeded(context);

		std::cout << "_";
		context.lastWasStyleEnd = false;
	}
}

void ensureStyleApplied(Context &context) {
	writeStyleDiff(context, context.currentStyle, context.currentStyles.top());
	context.currentStyle = context.currentStyles.top();
}

void setLastWasWhitespace(Context &context, const char c) {
	context.lastWasWhitespace = (c == ' ' || c == '\n');
}

void writeEscaped(Context &context, const char c) {
	if ((c == ' ') && (! options().edgeSpacesInsideBoldItalic)) {
		++(context.remainingSpacesCount);
	} else {
		ensureStyleApplied(context);

		::writeRemainingSpaces(context);
		::writeSpaceAfterStyleIfNeeded(context, c);

		switch (c) {
			case '<':
				std::cout << "&lt;";
				break;
			case '>':
				std::cout << "&gt;";
				break;
			case '.':
				if (options().escapeDotInText)
					std::cout << '\\';
				std::cout << c;
				break;
			case '\\':
			case '`':
			case '*':
			case '_':
			case '{':
			case '}':
			case '[':
			case ']':
			case '(':
			case ')':
			case '#':
			case '+':
			case '-':
			case '!':
				std::cout << '\\';
			default:
				std::cout << c;
		}
	}

	::setLastWasWhitespace(context, c);
}

void writeEscaped(Context &context, const std::string &str) {
	for (const char &c : str) {
		::writeEscaped(context, c);
	}
}

void writeRaw(Context &context, const char c) {
	ensureStyleApplied(context);

	::writeRemainingSpaces(context);
	::writeSpaceAfterStyleIfNeeded(context, c);

	std::cout << c;

	::setLastWasWhitespace(context, c);
}

void writeRaw(Context &context, const std::string &str) {
	for (const char &c : str) {
		::writeRaw(context, c);
	}
}

void pushStyle(Context &context, const Style &style) {
	context.currentStyles.push(context.currentStyles.top().apply(style));
}

void popStyle(Context &context) {
	context.currentStyles.pop();
}

void fixOutlineLevelStyleForMarkdown(OutlineLevelStyle &style, const bool numberFormats, const bool startValue, const bool levels) {
	if (! numberFormats) {
		if (! style.numFormat.empty()) {
			style.numFormat = "1";
		}

		style.numLetterSync = false;
		style.prefix = "";
		style.suffix = ".";
	}

	if (! startValue) {
		style.startValue = 1;
	}

	if (! levels) {
		style.displayLevels = 1;
	}
}

void onStart(void *userData, const XML_Char *name, const XML_Char **atts) {
	Context *context = static_cast<Context *>(userData);

	::processStyles_onStart(&context->stylesContext, name, atts);

	if (! ::strcmp(name, "text:h")) {
		uint32_t level = ::attrUint(atts, "text:outline-level", 0);

		::writeRaw(*context, std::string(std::max(level, static_cast<uint32_t>(1)), '#'));
		::writeRaw(*context, ' ');

		if (level > 0) {
			while (context->currentOutlineNumbering.size() > level) {
				context->currentOutlineNumbering.pop_back();
			}
			while (context->currentOutlineNumbering.size() < level - 1) {
				uint32_t addedLevel = static_cast<uint32_t>(context->currentOutlineNumbering.size()) + 1;
				context->currentOutlineNumbering.push_back(context->stylesContext.styles.getOutlineLevelStyle(addedLevel).startValue);
			}

			OutlineLevelStyle outlineLevelStyle = context->stylesContext.styles.getOutlineLevelStyle(level);
			::fixOutlineLevelStyleForMarkdown(outlineLevelStyle, options().headingNumberFormats, options().headingNumbersStartValue, options().headingNumbersLevels);

			if (context->currentOutlineNumbering.size() < level) {
				context->currentOutlineNumbering.push_back(outlineLevelStyle.startValue);
			} else {
				++(context->currentOutlineNumbering.back());
			}

			uint32_t currentNumber = context->currentOutlineNumbering.back();

			uint32_t fromLevel = 1;
			if (outlineLevelStyle.displayLevels <= level) {
				fromLevel = 1 + level - outlineLevelStyle.displayLevels;
			} else {
				std::cerr << "More levels to display than the current level: " << outlineLevelStyle.displayLevels << " > " << level << std::endl;
			}

			if (options().stylesInHeadingNumbers && (! outlineLevelStyle.styleName.empty())) {
				::pushStyle(*context, context->stylesContext.styles.getMergedStyle(outlineLevelStyle.styleName));
			}

			::writeEscaped(*context, outlineLevelStyle.prefix);

			for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
				OutlineLevelStyle higherLevelStyle = context->stylesContext.styles.getOutlineLevelStyle(higherLevel);
				::fixOutlineLevelStyleForMarkdown(higherLevelStyle, options().headingNumberFormats, options().headingNumbersStartValue, options().headingNumbersLevels);
				if (! higherLevelStyle.numFormat.empty()) {
					::writeEscaped(*context, numbering::createNumber(context->currentOutlineNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync));
				}
				if (! options().escapeDotInHeadingNumbers)
					::writeRaw(*context, '.');
				else
					::writeEscaped(*context, '.');
			}
			if (! outlineLevelStyle.numFormat.empty()) {
				::writeEscaped(*context, numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync));
			}

			if ((! options().escapeDotInHeadingNumbers) && outlineLevelStyle.suffix == ".")
				::writeRaw(*context, '.');
			else
				::writeEscaped(*context, outlineLevelStyle.suffix);

			if (options().stylesInHeadingNumbers && (! outlineLevelStyle.styleName.empty())) {
				::popStyle(*context);
			}

			::writeRaw(*context, ' ');
		}
	}
	if (! ::strcmp(name, "text:p")) {
		::pushStyle(*context, context->stylesContext.styles.getMergedStyle(::attrString(atts, "text:style-name", "")));
	}
	if (! ::strcmp(name, "text:span")) {
		::pushStyle(*context, context->stylesContext.styles.getMergedStyle(::attrString(atts, "text:style-name", "")));
	}
	if (! ::strcmp(name, "text:line-break")) {
		::writeRaw(*context, "  \n");
	}
	if (! ::strcmp(name, "text:list")) {
		std::string listStyleName = ::attrString(atts, "text:style-name", "");

		const bool newList = (! listStyleName.empty());

		if (newList) {
			context->currentLists.push(List());

			context->currentLists.top().listStyleName = ::attrString(atts, "text:style-name", "");
		}

		uint32_t level = static_cast<uint32_t>(context->currentLists.top().currentNumbering.size()) + 1;

		if (context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).isNumbered(level)) {
			const OutlineLevelStyle &outlineLevelStyle = context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).getOutlineLevelStyle(level);
			context->currentLists.top().currentNumbering.push_back(outlineLevelStyle.startValue);
		} else {
			context->currentLists.top().currentNumbering.push_back(0);
		}
	}
	if (! ::strcmp(name, "text:list-item")) {
		uint32_t level = static_cast<uint32_t>(context->currentLists.top().currentNumbering.size());

		::writeRaw(*context, std::string((level - 1) * 2, ' '));

		if (context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).isNumbered(level)) {
			OutlineLevelStyle outlineLevelStyle = context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).getOutlineLevelStyle(level);
			::fixOutlineLevelStyleForMarkdown(outlineLevelStyle, options().listNumberFormats, options().listNumbersStartValue, options().listNumbersLevels);

			uint32_t currentNumber = context->currentLists.top().currentNumbering.back();

			uint32_t fromLevel = 1;
			if (outlineLevelStyle.displayLevels <= level) {
				fromLevel = 1 + level - outlineLevelStyle.displayLevels;
			} else {
				std::cerr << "More levels to display than the current level: " << outlineLevelStyle.displayLevels << " > " << level << std::endl;
			}

			if (options().stylesInListNumbers && (! outlineLevelStyle.styleName.empty())) {
				::pushStyle(*context, context->stylesContext.styles.getMergedStyle(outlineLevelStyle.styleName));
			}

			::writeEscaped(*context, outlineLevelStyle.prefix);

			for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
				OutlineLevelStyle higherLevelStyle = context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).getOutlineLevelStyle(higherLevel);
				::fixOutlineLevelStyleForMarkdown(higherLevelStyle, options().listNumberFormats, options().listNumbersStartValue, options().listNumbersLevels);

				if (! higherLevelStyle.numFormat.empty()) {
					::writeEscaped(*context, numbering::createNumber(context->currentLists.top().currentNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync));
				}
				if (! options().escapeDotInListNumbers)
					::writeRaw(*context, '.');
				else
					::writeEscaped(*context, '.');
			}
			if (! outlineLevelStyle.numFormat.empty()) {
				::writeEscaped(*context, numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync));
			}

			if ((! options().escapeDotInListNumbers) && outlineLevelStyle.suffix == ".")
				::writeRaw(*context, '.');
			else
				::writeEscaped(*context, outlineLevelStyle.suffix);

			if (options().stylesInListNumbers && (! outlineLevelStyle.styleName.empty())) {
				::popStyle(*context);
			}
		} else {
			::writeRaw(*context, (level % 2) ? '*' : '-');
		}

		::writeRaw(*context, ' ');
	}

	if (! ::strcmp(name, "text:a")) {
		context->currentUrl = ::attrString(atts, "xlink:href", "");
		::writeRaw(*context, '[');
	}
}

void onEnd(void *userData, const XML_Char *name) {
	Context *context = static_cast<Context *>(userData);

	::processStyles_onEnd(&context->stylesContext, name);

	if (! ::strcmp(name, "text:h")) {
		::writeRaw(*context, '\n');
		::writeRaw(*context, '\n');
	}
	if (! ::strcmp(name, "text:p")) {
		::popStyle(*context);

		::writeRaw(*context, '\n');

		if (context->currentLists.size() == 0)
			::writeRaw(*context, '\n');
	}
	if (! ::strcmp(name, "text:span")) {
		::popStyle(*context);
	}
	if (! ::strcmp(name, "text:list")) {
		context->currentLists.top().currentNumbering.pop_back();

		if (context->currentLists.top().currentNumbering.empty()) {
			context->currentLists.pop();
		}

		if (context->currentLists.size() == 0)
			::writeRaw(*context, '\n');
	}
	if (! ::strcmp(name, "text:list-item")) {
		++context->currentLists.top().currentNumbering.back();
	}
	if (! ::strcmp(name, "text:a")) {
		::writeRaw(*context, ']');
		::writeRaw(*context, '(');
		::writeEscaped(*context, context->currentUrl);
		::writeRaw(*context, ')');
	}
}

void onData(void *userData, const XML_Char *s, int len) {
	Context *context = static_cast<Context *>(userData);

	for (const XML_Char *c = s; c < s + len; ++c) {
		::writeEscaped(*context, *c);
	}
}

}

void parseContent(zip_file *f, StylesContext &stylesContext) {
	Context context(stylesContext);

	XML_Parser parser = ::XML_ParserCreate(nullptr);
	BOOST_SCOPE_EXIT(&parser) {
		::XML_ParserFree(parser);
	} BOOST_SCOPE_EXIT_END

	::XML_SetUserData(parser, &context);
	::XML_SetElementHandler(parser, &onStart, &onEnd);
	::XML_SetCharacterDataHandler(parser, &onData);

	static const size_t bufferSize = 4 * 1024;
	char buffer[bufferSize];

	int64_t readResult = 0;

	while ((readResult = zip_fread(f, buffer, bufferSize)) > 0) {
		if (::XML_Parse(parser, buffer, static_cast<int>(readResult), false) == 0)
			throw 11;
	}

	if (readResult < 0) {
		throw 10;
	}

	if (::XML_Parse(parser, buffer, 0, true) == 0)
		throw 12;
}
