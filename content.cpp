#include "content.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>
#include <boost/scope_exit.hpp>
#include <expat.h>
#include "expat_utils.h"
#include "numbering.h"
#include "options.h"
#include "util.h"
#include "writer_output.h"

namespace {

void writeRemainingSpaces(ContentContext &context) {
	for (uint32_t i = 0; i < context.remainingSpacesCount; ++i)
		context.w.writeVisibleText_escaped(' ');

	if (context.remainingSpacesCount > 0) {
		context.lastWasStyleEnd = false;
	}

	context.remainingSpacesCount = 0;
}

void writeSpaceBeforeStyleIfNeeded(ContentContext &context) {
	if ((! options().boldItalicBeginInsideOfWords) && (! context.lastWasWhitespace)) {
		context.w.writeVisibleText_escaped(' ');
		context.lastWasWhitespace = true;
	}
}

void writeSpaceAfterStyleIfNeeded(ContentContext &context, const char currentChar) {
	if ((! options().boldItalicEndInsideOfWords) && (context.lastWasStyleEnd)) {
		if (currentChar != ' ' && currentChar != '\n')
			context.w.writeVisibleText_escaped(' ');
	}

	context.lastWasStyleEnd = false;
}

void writeStyleDiff(ContentContext &context, const Style &oldStyle, const Style &newStyle) {
	const bool enablingBold = options().bold && (!oldStyle.bold.get_value_or(false) && newStyle.bold.get_value_or(false));
	bool disablingBold = options().bold && (oldStyle.bold.get_value_or(false) && !newStyle.bold.get_value_or(false));
	const bool enablingItalic = options().italic && (!oldStyle.italic.get_value_or(false) && newStyle.italic.get_value_or(false));
	const bool disablingItalic = options().italic && (oldStyle.italic.get_value_or(false) && !newStyle.italic.get_value_or(false));

	const bool inItalic = oldStyle.italic.get_value_or(false) && (! disablingItalic);
	const bool inBold = oldStyle.bold.get_value_or(false) && (! disablingBold);

	if (disablingItalic) {
		if ((context.outerIsItalic) && (inBold || disablingBold)) {
			context.w.writeMarkup('*');
			context.w.writeMarkup('*');
			disablingBold = false;
		}

		context.w.writeMarkup('_');
		context.lastWasStyleEnd = true;

		if ((context.outerIsItalic) && (inBold)) {
			::writeRemainingSpaces(context);
			::writeSpaceBeforeStyleIfNeeded(context);

			context.w.writeMarkup('*');
			context.w.writeMarkup('*');
			context.lastWasStyleEnd = false;

			context.outerIsItalic = false;
		}
	}

	if (disablingBold) {
		if ((! context.outerIsItalic) && (inItalic)) {
			context.w.writeMarkup('_');
		}

		context.w.writeMarkup('*');
		context.w.writeMarkup('*');
		context.lastWasStyleEnd = true;

		if ((! context.outerIsItalic) && (inItalic)) {
			::writeRemainingSpaces(context);
			::writeSpaceBeforeStyleIfNeeded(context);

			context.w.writeMarkup('_');
			context.lastWasStyleEnd = false;

			context.outerIsItalic = true;
		}
	}

	if (enablingBold) {
		if ((! inItalic) || enablingItalic)
			context.outerIsItalic = false;

		::writeRemainingSpaces(context);
		::writeSpaceBeforeStyleIfNeeded(context);

		context.w.writeMarkup('*');
		context.w.writeMarkup('*');
		context.lastWasStyleEnd = false;
	}

	if (enablingItalic) {
		if ((! inBold) && (! enablingBold))
			context.outerIsItalic = true;

		::writeRemainingSpaces(context);
		::writeSpaceBeforeStyleIfNeeded(context);

		context.w.writeMarkup('_');
		context.lastWasStyleEnd = false;
	}
}

void ensureStyleApplied(ContentContext &context) {
	writeStyleDiff(context, context.currentStyle, context.currentStyles.top());
	context.currentStyle = context.currentStyles.top();
}

void setLastWasWhitespace(ContentContext &context, const char c) {
	context.lastWasWhitespace = (c == ' ' || c == '\n');
}

void writeMarkup(ContentContext &context, const char c) {
	ensureStyleApplied(context);

	::writeRemainingSpaces(context);
	::writeSpaceAfterStyleIfNeeded(context, c);

	context.w.writeMarkup(c);

	::setLastWasWhitespace(context, c);
}

void writeMarkup(ContentContext &context, const std::string &str) {
	for (const char &c : str) {
		::writeMarkup(context, c);
	}
}

void writeVisibleText(ContentContext &context, const char c) {
	ensureStyleApplied(context);

	::writeRemainingSpaces(context);
	::writeSpaceAfterStyleIfNeeded(context, c);

	context.w.writeVisibleText(c);

	::setLastWasWhitespace(context, c);
}

void writeVisibleText(ContentContext &context, const std::string &str) {
	for (const char &c : str) {
		::writeVisibleText(context, c);
	}
}

void writeVisibleText_escaped(ContentContext &context, const char c) {
	if ((c == ' ') && (! options().edgeSpacesInsideBoldItalic)) {
		++(context.remainingSpacesCount);
	} else {
		ensureStyleApplied(context);

		::writeRemainingSpaces(context);
		::writeSpaceAfterStyleIfNeeded(context, c);

		context.w.writeVisibleText_escaped(c);
	}

	::setLastWasWhitespace(context, c);
}

void writeVisibleText_escaped(ContentContext &context, const std::string &str) {
	for (const char &c : str) {
		::writeVisibleText_escaped(context, c);
	}
}

void pushStyle(ContentContext &context, const Style &style) {
	context.currentStyles.push(context.currentStyles.top().apply(style));
}

void popStyle(ContentContext &context) {
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

}

char hexDigit(const char digit) {
	if (digit < 10) {
		return '0' + digit;
	} else {
		return 'A' + (digit - 10);
	}
}

std::string transformBookmarkText(const std::string &str) {
	std::ostringstream result;

	bool lastWasDash = false;

	for (const char &c : str) {
		const bool isFirst128 = ((c & 0x80) == 0);

		if (isFirst128) {
			if (
				((c >= 'a') && (c <= 'z'))
				||
				((c >= '0') && (c <= '9'))
				||
				(c == '_')
			) {
				result << c;
				lastWasDash = false;
			}
			else if ((c >= 'A') && (c <= 'Z')) {
				result << static_cast<char>(::tolower(c));
				lastWasDash = false;
			}
			else {
				if (! lastWasDash) {
					result << '-';
					lastWasDash = true;
				}
			}
		} else {
			result << '%' << ::hexDigit((c < 0 ? c+ 256 : c) / 16) << ::hexDigit((c < 0 ? c+ 256 : c) % 16);
			lastWasDash = false;
		}
	}

	return result.str();
}

ContentHandler::ContentHandler(const Structure &structure, const Styles &styles, ContentContext &context)
	: structure(structure)
	, styles(styles)
	, context(context)
{
}

void ContentHandler::onStart(const XML_Char *name, const XML_Char **atts) {
	if (! ::strcmp(name, "text:h")) {
		uint32_t level = ::attrUint(atts, "text:outline-level", 0);
		context.currentHeadingOutlineLevel = level;

		if (level > 0) {
			context.underlineHeadingUsing = '\0';
			if (options().underlineHeading1 && level == 1) {
				context.underlineHeadingUsing = '=';
			}
			if (options().underlineHeading2 && level == 2) {
				context.underlineHeadingUsing = '-';
			}

			if (context.underlineHeadingUsing == '\0') {
				::writeMarkup(context, std::string(level, '#'));
				::writeMarkup(context, ' ');
			}

			if (Writer_Output *writerOutput = dynamic_cast<Writer_Output *>(&context.w))
				writerOutput->resetCodePointsCount();

			while (context.currentOutlineNumbering.size() > level) {
				context.currentOutlineNumbering.pop_back();
			}
			while (context.currentOutlineNumbering.size() < level - 1) {
				uint32_t addedLevel = static_cast<uint32_t>(context.currentOutlineNumbering.size()) + 1;
				context.currentOutlineNumbering.push_back(styles.getOutlineLevelStyle(addedLevel).startValue);
			}

			OutlineLevelStyle outlineLevelStyle = styles.getOutlineLevelStyle(level);
			::fixOutlineLevelStyleForMarkdown(outlineLevelStyle, options().headingNumberFormats, options().headingNumbersStartValue, options().headingNumbersLevels);

			if (context.currentOutlineNumbering.size() < level) {
				context.currentOutlineNumbering.push_back(outlineLevelStyle.startValue);
			} else {
				++(context.currentOutlineNumbering.back());
			}

			if (! outlineLevelStyle.numFormat.empty()) {
				uint32_t currentNumber = context.currentOutlineNumbering.back();

				uint32_t fromLevel = 1;
				if (outlineLevelStyle.displayLevels <= level) {
					fromLevel = 1 + level - outlineLevelStyle.displayLevels;
				} else {
					std::cerr << "More levels to display than the current level: " << outlineLevelStyle.displayLevels << " > " << level << std::endl;
				}

				if (options().stylesInHeadingNumbers && (! outlineLevelStyle.styleName.empty())) {
					::pushStyle(context, styles.getMergedStyle(outlineLevelStyle.styleName));
				}

				::writeVisibleText_escaped(context, outlineLevelStyle.prefix);

				for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
					OutlineLevelStyle higherLevelStyle = styles.getOutlineLevelStyle(higherLevel);
					::fixOutlineLevelStyleForMarkdown(higherLevelStyle, options().headingNumberFormats, options().headingNumbersStartValue, options().headingNumbersLevels);
					if (! higherLevelStyle.numFormat.empty()) {
						const std::string numberText = numbering::createNumber(context.currentOutlineNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync);
						::writeVisibleText_escaped(context, numberText);

						if (! options().escapeDotInHeadingNumbers)
							::writeVisibleText(context, '.');
						else
							::writeVisibleText_escaped(context, '.');
					}
				}
				if (! outlineLevelStyle.numFormat.empty()) {
					const std::string numberText = numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync);
					::writeVisibleText_escaped(context, numberText);
				}

				if ((! options().escapeDotInHeadingNumbers) && outlineLevelStyle.suffix == ".")
					::writeVisibleText(context, '.');
				else
					::writeVisibleText_escaped(context, outlineLevelStyle.suffix);

				if (options().stylesInHeadingNumbers && (! outlineLevelStyle.styleName.empty())) {
					::popStyle(context);
				}

				::writeVisibleText(context, ' ');
			}
		}
	}
	if (! ::strcmp(name, "text:p")) {
		::pushStyle(context, styles.getMergedStyle(::attrString(atts, "text:style-name", "")));
	}
	if (! ::strcmp(name, "text:span")) {
		::pushStyle(context, styles.getMergedStyle(::attrString(atts, "text:style-name", "")));
	}
	if (! ::strcmp(name, "text:line-break")) {
		::writeMarkup(context, "  ");
		::writeVisibleText(context, '\n');
	}
	if (! ::strcmp(name, "text:list")) {
		std::string listStyleName = ::attrString(atts, "text:style-name", "");

		const bool newList = (! listStyleName.empty());

		if (newList) {
			context.currentLists.push(List());

			context.currentLists.top().listStyleName = ::attrString(atts, "text:style-name", "");
		}

		uint32_t level = static_cast<uint32_t>(context.currentLists.top().currentNumbering.size()) + 1;

		if (styles.getListStyle(context.currentLists.top().listStyleName).isNumbered(level)) {
			OutlineLevelStyle outlineLevelStyle = styles.getListStyle(context.currentLists.top().listStyleName).getOutlineLevelStyle(level);
			::fixOutlineLevelStyleForMarkdown(outlineLevelStyle, options().listNumberFormats, options().listNumbersStartValue, options().listNumbersLevels);
			context.currentLists.top().currentNumbering.push_back(outlineLevelStyle.startValue);
		} else {
			context.currentLists.top().currentNumbering.push_back(0);
		}
	}
	if (! ::strcmp(name, "text:list-item")) {
		uint32_t level = static_cast<uint32_t>(context.currentLists.top().currentNumbering.size());

		::writeMarkup(context, std::string((level - 1) * 2, ' '));

		if (styles.getListStyle(context.currentLists.top().listStyleName).isNumbered(level)) {
			OutlineLevelStyle outlineLevelStyle = styles.getListStyle(context.currentLists.top().listStyleName).getOutlineLevelStyle(level);
			::fixOutlineLevelStyleForMarkdown(outlineLevelStyle, options().listNumberFormats, options().listNumbersStartValue, options().listNumbersLevels);

			uint32_t currentNumber = context.currentLists.top().currentNumbering.back();

			uint32_t fromLevel = 1;
			if (outlineLevelStyle.displayLevels <= level) {
				fromLevel = 1 + level - outlineLevelStyle.displayLevels;
			} else {
				std::cerr << "More levels to display than the current level: " << outlineLevelStyle.displayLevels << " > " << level << std::endl;
			}

			if (options().stylesInListNumbers && (! outlineLevelStyle.styleName.empty())) {
				::pushStyle(context, styles.getMergedStyle(outlineLevelStyle.styleName));
			}

			::writeVisibleText_escaped(context, outlineLevelStyle.prefix);

			for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
				OutlineLevelStyle higherLevelStyle = styles.getListStyle(context.currentLists.top().listStyleName).getOutlineLevelStyle(higherLevel);
				::fixOutlineLevelStyleForMarkdown(higherLevelStyle, options().listNumberFormats, options().listNumbersStartValue, options().listNumbersLevels);

				if (! higherLevelStyle.numFormat.empty()) {
					::writeVisibleText_escaped(context, numbering::createNumber(context.currentLists.top().currentNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync));

					if (! options().escapeDotInListNumbers)
						::writeVisibleText(context, '.');
					else
						::writeVisibleText_escaped(context, '.');
				}
			}
			if (! outlineLevelStyle.numFormat.empty()) {
				::writeVisibleText_escaped(context, numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync));
			}

			if ((! options().escapeDotInListNumbers) && outlineLevelStyle.suffix == ".")
				::writeVisibleText(context, '.');
			else
				::writeVisibleText_escaped(context, outlineLevelStyle.suffix);

			if (options().stylesInListNumbers && (! outlineLevelStyle.styleName.empty())) {
				::popStyle(context);
			}
		} else {
			::writeMarkup(context, (level % 2) ? '*' : '-');
		}

		::writeVisibleText(context, ' ');
	}

	if (! ::strcmp(name, "text:a")) {
		context.currentUrl = ::attrString(atts, "xlink:href", "");
		::writeMarkup(context, '[');
	}
	if (! ::strcmp(name, "text:bookmark-ref")) {
		if (options().linksToHeadings) {
			const std::string bookmarkName = ::attrString(atts, "text:ref-name", "");
			if (! bookmarkName.empty()) {
				const Structure::NameToText::const_iterator it = structure.bookmarks.find(bookmarkName);
				if (it != structure.bookmarks.end()) {
					context.currentUrl = '#' + ::transformBookmarkText(it->second);
					::writeMarkup(context, '[');
				} else {
					std::cerr << "Bookmark not found: " << bookmarkName << std::endl;
				}
			}
		}
	}
}

void ContentHandler::onEnd(const XML_Char *name) {
	if (! ::strcmp(name, "text:h")) {
		if (context.underlineHeadingUsing == '\0') {
			if (options().closeHeadings || options().closeHeadingsShort) {
				const uint32_t level = context.currentHeadingOutlineLevel;

				::writeMarkup(context, ' ');
				::writeMarkup(context, std::string(options().closeHeadingsShort ? 1 : level, '#'));
			}
		}
		if (context.underlineHeadingUsing != '\0') {
			::writeMarkup(context, '\n');

			if (Writer_Output *writerOutput = dynamic_cast<Writer_Output *>(&context.w))
				::writeMarkup(context, std::string(writerOutput->getCodePointsCount() - 1, context.underlineHeadingUsing));
			else
				::writeMarkup(context, std::string(10, context.underlineHeadingUsing));
		}
		::writeVisibleText(context, '\n');
		::writeMarkup(context, '\n');
	}
	if (! ::strcmp(name, "text:p")) {
		::popStyle(context);

		::writeVisibleText(context, '\n');

		if (context.currentLists.size() == 0)
			::writeMarkup(context, '\n');
	}
	if (! ::strcmp(name, "text:span")) {
		::popStyle(context);
	}
	if (! ::strcmp(name, "text:list")) {
		context.currentLists.top().currentNumbering.pop_back();

		if (context.currentLists.top().currentNumbering.empty()) {
			context.currentLists.pop();
		}

		if (context.currentLists.size() == 0)
			::writeMarkup(context, '\n');
	}
	if (! ::strcmp(name, "text:list-item")) {
		++context.currentLists.top().currentNumbering.back();
	}
	if ((! ::strcmp(name, "text:a")) || (! ::strcmp(name, "text:bookmark-ref"))) {
		if (! context.currentUrl.empty()) {
			::writeMarkup(context, ']');
			::writeMarkup(context, '(');
			::writeVisibleText_escaped(context, context.currentUrl);
			::writeMarkup(context, ')');

			context.currentUrl = "";
		}
	}
}

void ContentHandler::onData(const XML_Char *s, int len) {
	::writeVisibleText_escaped(context, std::string(s, static_cast<size_t>(len)));
}
