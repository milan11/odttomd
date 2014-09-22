#include "styles.h"

#include <iostream>
#include <boost/scope_exit.hpp>
#include <expat.h>
#include "expat_utils.h"
#include "util.h"

namespace {

void onStart(void *userData, const XML_Char *name, const XML_Char **atts) {
	StylesContext *context = static_cast<StylesContext *>(userData);

	::processStyles_onStart(context, name, atts);
}

void onEnd(void *userData, const XML_Char *name) {
	StylesContext *context = static_cast<StylesContext *>(userData);

	::processStyles_onEnd(context, name);
}

void readOutlineLevelStyle(const XML_Char **atts, OutlineLevelStyle &outlineLevelStyle) {
	outlineLevelStyle.numFormat = ::attrString(atts, "style:num-format", "");
	outlineLevelStyle.numLetterSync = ::attrBool(atts, "style:num-letter-sync", false);
	outlineLevelStyle.prefix = ::attrString(atts, "style:num-prefix", "");
	outlineLevelStyle.suffix = ::attrString(atts, "style:num-suffix", "");
	outlineLevelStyle.startValue = ::attrUint(atts, "text:start-value", 1);
	outlineLevelStyle.displayLevels = ::attrUint(atts, "text:display-levels", 1);
	outlineLevelStyle.styleName = ::attrString(atts, "text:style-name", "");
}

}

void processStyles_onStart(StylesContext *context, const XML_Char *name, const XML_Char **atts) {
	if (! ::strcmp(name, "style:style")) {
		context->currentStyle = &context->styles.styles[::attrString(atts, "style:name", "")];
		context->currentStyle->parentStyleName = ::attrString(atts, "style:parent-style-name", "");
	}
	if (! ::strcmp(name, "style:text-properties")) {
		if (::attrString(atts, "fo:font-weight", "") == "normal") {
			context->currentStyle->bold = false;
		}
		if (::attrString(atts, "fo:font-weight", "") == "bold") {
			context->currentStyle->bold = true;
		}

		if (::attrString(atts, "fo:font-style", "") == "normal") {
			context->currentStyle->italic = false;
		}
		if (::attrString(atts, "fo:font-style", "") == "italic") {
			context->currentStyle->italic = true;
		}
	}

	if (! ::strcmp(name, "text:outline-level-style")) {
		uint32_t level = ::attrUint(atts, "text:level", 0);
		if (level != 0) {
			OutlineLevelStyle &outlineLevelStyle = context->styles.outlineLevelStyles[level];
			readOutlineLevelStyle(atts, outlineLevelStyle);
		}
	}

	if (! ::strcmp(name, "text:list-style")) {
		context->currentListStyle = &context->styles.listStyles[::attrString(atts, "style:name", "")];
	}

	if (! ::strcmp(name, "text:list-level-style-number")) {
		uint32_t level = ::attrUint(atts, "text:level", 0);
		if (level != 0) {
			OutlineLevelStyle &outlineLevelStyle = context->currentListStyle->outlineLevelStyles[level];
			readOutlineLevelStyle(atts, outlineLevelStyle);
		}
	}

	if (! ::strcmp(name, "text:list-level-style-bullet")) {
		uint32_t level = ::attrUint(atts, "text:level", 0);
		if (level != 0) {
			BulletStyle &bulletStyle = context->currentListStyle->bulletStyles[level];
			(void)bulletStyle;
		}
	}
}

void processStyles_onEnd(StylesContext *context, const XML_Char *name) {
	if (! ::strcmp(name, "style:style")) {
		context->currentStyle = nullptr;
	}
	if (! ::strcmp(name, "text:list-style")) {
		context->currentListStyle = nullptr;
	}
}

StylesContext parseStyles(zip_file *f) {
	StylesContext context;

	XML_Parser parser = ::XML_ParserCreate(nullptr);
	BOOST_SCOPE_EXIT(&parser) {
		::XML_ParserFree(parser);
	} BOOST_SCOPE_EXIT_END

	::XML_SetUserData(parser, &context);
	::XML_SetElementHandler(parser, &onStart, &onEnd);

	static const size_t bufferSize = 4 * 1024;
	char buffer[bufferSize];

	int64_t readResult = 0;

	while ((readResult = zip_fread(f, buffer, bufferSize)) > 0) {
		if (::XML_Parse(parser, buffer, static_cast<int>(readResult), false) == 0)
			throw 21;
	}

	if (readResult < 0) {
		throw 20;
	}

	if (::XML_Parse(parser, buffer, 0, true) == 0)
		throw 22;

	return context;
}
