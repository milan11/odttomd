#include "content.h"

#include <algorithm>
#include <vector>
#include <boost/scope_exit.hpp>
#include <expat.h>
#include "expat_utils.h"
#include "numbering.h"
#include "options.h"
#include "util.h"

namespace {

void writeRemainingSpaces(ContentContext &context) {
	context.w.write(std::string(context.remainingSpacesCount, ' '));

	if (context.remainingSpacesCount > 0) {
		context.lastWasStyleEnd = false;
	}

	context.remainingSpacesCount = 0;
}

void writeSpaceBeforeStyleIfNeeded(ContentContext &context) {
	if ((! options().boldItalicBeginInsideOfWords) && (! context.lastWasWhitespace)) {
		context.w.write(' ');
		context.lastWasWhitespace = true;
	}
}

void writeSpaceAfterStyleIfNeeded(ContentContext &context, const char currentChar) {
	if ((! options().boldItalicEndInsideOfWords) && (context.lastWasStyleEnd)) {
		if (currentChar != ' ' && currentChar != '\n')
			context.w.write(' ');
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
			context.w.write("**");
			disablingBold = false;
		}

		context.w.write("_");
		context.lastWasStyleEnd = true;

		if ((context.outerIsItalic) && (inBold)) {
			::writeRemainingSpaces(context);
			::writeSpaceBeforeStyleIfNeeded(context);

			context.w.write("**");
			context.lastWasStyleEnd = false;

			context.outerIsItalic = false;
		}
	}

	if (disablingBold) {
		if ((! context.outerIsItalic) && (inItalic)) {
			context.w.write("_");
		}

		context.w.write("**");
		context.lastWasStyleEnd = true;

		if ((! context.outerIsItalic) && (inItalic)) {
			::writeRemainingSpaces(context);
			::writeSpaceBeforeStyleIfNeeded(context);

			context.w.write("_");
			context.lastWasStyleEnd = false;

			context.outerIsItalic = true;
		}
	}

	if (enablingBold) {
		if ((! inItalic) || enablingItalic)
			context.outerIsItalic = false;

		::writeRemainingSpaces(context);
		::writeSpaceBeforeStyleIfNeeded(context);

		context.w.write("**");
		context.lastWasStyleEnd = false;
	}

	if (enablingItalic) {
		if ((! inBold) && (! enablingBold))
			context.outerIsItalic = true;

		::writeRemainingSpaces(context);
		::writeSpaceBeforeStyleIfNeeded(context);

		context.w.write("_");
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

void writeEscaped(ContentContext &context, const char c) {
	if ((c == ' ') && (! options().edgeSpacesInsideBoldItalic)) {
		++(context.remainingSpacesCount);
	} else {
		ensureStyleApplied(context);

		::writeRemainingSpaces(context);
		::writeSpaceAfterStyleIfNeeded(context, c);

		if (options().escapeInText.find(c) != std::string::npos) {
			context.w.write('\\');
			context.w.write(c);
		}
		else if (options().entitiesInText.find(c) != std::string::npos) {
			switch (c) {
				case '<':
					context.w.write("&lt;");
					break;
				case '>':
					context.w.write("&gt;");
					break;
				case '"':
					context.w.write("&quot;");
					break;
				case '&':
					context.w.write("&amp;");
					break;
				case '\'':
					context.w.write("&apos;");
					break;
				default:
					context.w.write(c);
			}
		}
		else {
			context.w.write(c);
		}
	}

	::setLastWasWhitespace(context, c);
}

void writeEscaped(ContentContext &context, const std::string &str) {
	for (const char &c : str) {
		::writeEscaped(context, c);
	}
}

void writeRaw(ContentContext &context, const char c) {
	ensureStyleApplied(context);

	::writeRemainingSpaces(context);
	::writeSpaceAfterStyleIfNeeded(context, c);

	context.w.write(c);

	::setLastWasWhitespace(context, c);
}

void writeRaw(ContentContext &context, const std::string &str) {
	for (const char &c : str) {
		::writeRaw(context, c);
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

ContentHandler::ContentHandler(ContentContext &context, const Styles &styles)
	: context(context)
	, styles(styles)
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
				::writeRaw(context, std::string(level, '#'));
				::writeRaw(context, ' ');
			}

			context.w.resetCodePointsCount();

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

				::writeEscaped(context, outlineLevelStyle.prefix);

				for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
					OutlineLevelStyle higherLevelStyle = styles.getOutlineLevelStyle(higherLevel);
					::fixOutlineLevelStyleForMarkdown(higherLevelStyle, options().headingNumberFormats, options().headingNumbersStartValue, options().headingNumbersLevels);
					if (! higherLevelStyle.numFormat.empty()) {
						const std::string numberText = numbering::createNumber(context.currentOutlineNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync);
						::writeEscaped(context, numberText);

						if (! options().escapeDotInHeadingNumbers)
							::writeRaw(context, '.');
						else
							::writeEscaped(context, '.');
					}
				}
				if (! outlineLevelStyle.numFormat.empty()) {
					const std::string numberText = numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync);
					::writeEscaped(context, numberText);
				}

				if ((! options().escapeDotInHeadingNumbers) && outlineLevelStyle.suffix == ".")
					::writeRaw(context, '.');
				else
					::writeEscaped(context, outlineLevelStyle.suffix);

				if (options().stylesInHeadingNumbers && (! outlineLevelStyle.styleName.empty())) {
					::popStyle(context);
				}

				::writeRaw(context, ' ');
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
		::writeRaw(context, "  \n");
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

		::writeRaw(context, std::string((level - 1) * 2, ' '));

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

			::writeEscaped(context, outlineLevelStyle.prefix);

			for (uint32_t higherLevel = fromLevel; higherLevel < level; ++higherLevel) {
				OutlineLevelStyle higherLevelStyle = styles.getListStyle(context.currentLists.top().listStyleName).getOutlineLevelStyle(higherLevel);
				::fixOutlineLevelStyleForMarkdown(higherLevelStyle, options().listNumberFormats, options().listNumbersStartValue, options().listNumbersLevels);

				if (! higherLevelStyle.numFormat.empty()) {
					::writeEscaped(context, numbering::createNumber(context.currentLists.top().currentNumbering[higherLevel - 1], higherLevelStyle.numFormat, higherLevelStyle.numLetterSync));

					if (! options().escapeDotInListNumbers)
						::writeRaw(context, '.');
					else
						::writeEscaped(context, '.');
				}
			}
			if (! outlineLevelStyle.numFormat.empty()) {
				::writeEscaped(context, numbering::createNumber(currentNumber, outlineLevelStyle.numFormat, outlineLevelStyle.numLetterSync));
			}

			if ((! options().escapeDotInListNumbers) && outlineLevelStyle.suffix == ".")
				::writeRaw(context, '.');
			else
				::writeEscaped(context, outlineLevelStyle.suffix);

			if (options().stylesInListNumbers && (! outlineLevelStyle.styleName.empty())) {
				::popStyle(context);
			}
		} else {
			::writeRaw(context, (level % 2) ? '*' : '-');
		}

		::writeRaw(context, ' ');
	}

	if (! ::strcmp(name, "text:a")) {
		context.currentUrl = ::attrString(atts, "xlink:href", "");
		::writeRaw(context, '[');
	}
}

void ContentHandler::onEnd(const XML_Char *name) {
	if (! ::strcmp(name, "text:h")) {
		if (context.underlineHeadingUsing == '\0') {
			if (options().closeHeadings || options().closeHeadingsShort) {
				const uint32_t level = context.currentHeadingOutlineLevel;

				::writeRaw(context, ' ');
				::writeRaw(context, std::string(options().closeHeadingsShort ? 1 : level, '#'));
			}
		}
		if (context.underlineHeadingUsing != '\0') {
			::writeRaw(context, '\n');
			::writeRaw(context, std::string(context.w.getCodePointsCount() - 1, context.underlineHeadingUsing));
		}
		::writeRaw(context, '\n');
		::writeRaw(context, '\n');
	}
	if (! ::strcmp(name, "text:p")) {
		::popStyle(context);

		::writeRaw(context, '\n');

		if (context.currentLists.size() == 0)
			::writeRaw(context, '\n');
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
			::writeRaw(context, '\n');
	}
	if (! ::strcmp(name, "text:list-item")) {
		++context.currentLists.top().currentNumbering.back();
	}
	if (! ::strcmp(name, "text:a")) {
		::writeRaw(context, ']');
		::writeRaw(context, '(');
		::writeEscaped(context, context.currentUrl);
		::writeRaw(context, ')');
	}
}

void ContentHandler::onData(const XML_Char *s, int len) {
	for (const XML_Char *c = s; c < s + len; ++c) {
		::writeEscaped(context, *c);
	}
}
