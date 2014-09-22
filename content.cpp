#include "content.h"

#include <algorithm>
#include <iostream>
#include <stack>
#include <vector>
#include <boost/scope_exit.hpp>
#include <expat.h>
#include "expat_utils.h"
#include "numbering.h"
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
	std::string currentUrl;
	std::vector<uint32_t> currentOutlineNumbering;

	std::stack<List> currentLists;

};

void writeEscapedChar(const char c) {
	switch (c) {
		case '<':
			std::cout << "&lt;";
			break;
		case '>':
			std::cout << "&gt;";
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
		case '.':
		case '!':
			std::cout << '\\';
		default:
			std::cout << c;
	}
}

void writeEscapedString(const std::string &str) {
	for (const char &c : str) {
		::writeEscapedChar(c);
	}
}

void writeStyleDiff(const Style &oldStyle, const Style &newStyle) {
	if (oldStyle.bold.get_value_or(false) != newStyle.bold.get_value_or(false)) {
		if (oldStyle.italic.get_value_or(false)) {
			std::cout << "_";
		}
		std::cout << "**";
		if (newStyle.italic.get_value_or(false)) {
			std::cout << "_";
		}
	}
	else if (oldStyle.italic.get_value_or(false) != newStyle.italic.get_value_or(false)) {
		std::cout << "_";
	}
}

void pushStyle(Context &context, const Style &style) {
	const Style &oldStyle = context.currentStyles.top();
	context.currentStyles.push(context.currentStyles.top().apply(style));

	::writeStyleDiff(oldStyle, context.currentStyles.top());
}

void popStyle(Context &context) {
	const Style oldStyle = context.currentStyles.top();
	context.currentStyles.pop();

	::writeStyleDiff(oldStyle, context.currentStyles.top());
}

void onStart(void *userData, const XML_Char *name, const XML_Char **atts) {
	Context *context = static_cast<Context *>(userData);

	::processStyles_onStart(&context->stylesContext, name, atts);

	if (! ::strcmp(name, "text:h")) {
		uint32_t level = ::attrUint(atts, "text:outline-level", 0);

		std::cout << std::string(std::max(level, static_cast<uint32_t>(1)), '#') << ' ';

		if (level > 0) {
			while (context->currentOutlineNumbering.size() > level) {
				context->currentOutlineNumbering.pop_back();
			}
			while (context->currentOutlineNumbering.size() < level - 1) {
				uint32_t addedLevel = static_cast<uint32_t>(context->currentOutlineNumbering.size()) + 1;
				context->currentOutlineNumbering.push_back(context->stylesContext.styles.getOutlineLevelStyle(addedLevel).startValue);
			}

			const OutlineLevelStyle &outlineLevelStyle = context->stylesContext.styles.getOutlineLevelStyle(level);
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

			if (! outlineLevelStyle.styleName.empty()) {
				::pushStyle(*context, context->stylesContext.styles.getMergedStyle(outlineLevelStyle.styleName));
			}

			::writeEscapedString(outlineLevelStyle.prefix);

			for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
				const OutlineLevelStyle &higherLevelStyle = context->stylesContext.styles.getOutlineLevelStyle(higherLevel);
				if (! higherLevelStyle.numFormat.empty()) {
					::writeEscapedString(numbering::createNumber(context->currentOutlineNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync));
				}
				::writeEscapedChar('.');
			}
			if (! outlineLevelStyle.numFormat.empty()) {
				::writeEscapedString(numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync));
			}

			::writeEscapedString(outlineLevelStyle.suffix);

			if (! outlineLevelStyle.styleName.empty()) {
				::popStyle(*context);
			}

			std::cout << ' ';
		}
	}
	if (! ::strcmp(name, "text:p")) {
		::pushStyle(*context, context->stylesContext.styles.getMergedStyle(::attrString(atts, "text:style-name", "")));
	}
	if (! ::strcmp(name, "text:span")) {
		::pushStyle(*context, context->stylesContext.styles.getMergedStyle(::attrString(atts, "text:style-name", "")));
	}
	if (! ::strcmp(name, "text:line-break")) {
		std::cout << "  \n";
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

		std::cout << std::string((level - 1) * 2, ' ');

		if (context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).isNumbered(level)) {
			const OutlineLevelStyle &outlineLevelStyle = context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).getOutlineLevelStyle(level);

			uint32_t currentNumber = context->currentLists.top().currentNumbering.back();

			uint32_t fromLevel = 1;
			if (outlineLevelStyle.displayLevels <= level) {
				fromLevel = 1 + level - outlineLevelStyle.displayLevels;
			} else {
				std::cerr << "More levels to display than the current level: " << outlineLevelStyle.displayLevels << " > " << level << std::endl;
			}

			if (! outlineLevelStyle.styleName.empty()) {
				::pushStyle(*context, context->stylesContext.styles.getMergedStyle(outlineLevelStyle.styleName));
			}

			::writeEscapedString(outlineLevelStyle.prefix);

			for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
				const OutlineLevelStyle &higherLevelStyle = context->stylesContext.styles.getListStyle(context->currentLists.top().listStyleName).getOutlineLevelStyle(higherLevel);
				if (! higherLevelStyle.numFormat.empty()) {
					::writeEscapedString(numbering::createNumber(context->currentLists.top().currentNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync));
				}
				::writeEscapedChar('.');
			}
			if (! outlineLevelStyle.numFormat.empty()) {
				::writeEscapedString(numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync));
			}

			::writeEscapedString(outlineLevelStyle.suffix);

			if (! outlineLevelStyle.styleName.empty()) {
				::popStyle(*context);
			}
		} else {
			std::cout << ((level % 2) ? '*' : '-');
		}

		std::cout << ' ';
	}

	if (! ::strcmp(name, "text:a")) {
		context->currentUrl = ::attrString(atts, "xlink:href", "");
		std::cout << '[';
	}
}

void onEnd(void *userData, const XML_Char *name) {
	Context *context = static_cast<Context *>(userData);

	::processStyles_onEnd(&context->stylesContext, name);

	if (! ::strcmp(name, "text:h")) {
		std::cout << '\n';
		std::cout << '\n';
	}
	if (! ::strcmp(name, "text:p")) {
		::popStyle(*context);

		std::cout << '\n';

		if (context->currentLists.size() == 0)
			std::cout << '\n';
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
			std::cout << '\n';
	}
	if (! ::strcmp(name, "text:list-item")) {
		++context->currentLists.top().currentNumbering.back();
	}
	if (! ::strcmp(name, "text:a")) {
		std::cout << ']';
		std::cout << '(';
		::writeEscapedString(context->currentUrl);
		std::cout << ')';
	}
}

void onData(void *, const XML_Char *s, int len) {
	for (const XML_Char *c = s; c < s + len; ++c) {
		::writeEscapedChar(*c);
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
