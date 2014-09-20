#include "styles.h"

#include <iostream>
#include <boost/scope_exit.hpp>
#include <expat.h>
#include "expat_utils.h"
#include "util.h"

namespace {

class Context {

public:
	Styles styles;

	Style *current = nullptr;

};

void onStart(void *userData, const XML_Char *name, const XML_Char **atts) {
	Context *context = static_cast<Context *>(userData);

	if (! ::strcmp(name, "style:style")) {
		context->current = &context->styles.styles[::attrString(atts, "style:name", "")];
	}
	if (! ::strcmp(name, "style:text-properties")) {
		if (::attrString(atts, "fo:font-weight", "") == "bold") {
			context->current->bold = true;
		}
	}

	if (! ::strcmp(name, "text:outline-level-style")) {
		uint32_t level = ::attrUint(atts, "text:level", 0);
		if (level != 0) {
			OutlineLevelStyle &outlineLevelStyle = context->styles.outlineLevelStyles[level];
			outlineLevelStyle.numFormat = ::attrString(atts, "style:num-format", "");
			outlineLevelStyle.numLetterSync = ::attrBool(atts, "style:num-letter-sync", false);
			outlineLevelStyle.prefix = ::attrString(atts, "style:num-prefix", "");
			outlineLevelStyle.suffix = ::attrString(atts, "style:num-suffix", "");
			outlineLevelStyle.startValue = ::attrUint(atts, "text:start-value", 1);
		}
	}
}

void onEnd(void *userData, const XML_Char *name) {
	Context *context = static_cast<Context *>(userData);

	if (! ::strcmp(name, "style:style")) {
		context->current = nullptr;
	}
}

void onData(void *, const XML_Char *s, int len) {
	std::cout << std::string(s, static_cast<std::string::size_type>(len));
}

}

Styles parseStyles(zip_file *f) {
	Context context;

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
			throw 21;
	}

	if (readResult < 0) {
		throw 20;
	}

	if (::XML_Parse(parser, buffer, 0, true) == 0)
		throw 22;

	return context.styles;
}
