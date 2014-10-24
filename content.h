#pragma once

#include <stack>
#include "structure.h"
#include "styles.h"
#include "writer_output.h"
#include "xmlInZip.h"

class List {

public:
	std::string listStyleName;
	std::vector<uint32_t> currentNumbering;
};

class ContentContext {

public:
	ContentContext()
		: w(std::cout)
	{
		currentStyles.push(Style());
	}

public:
	Writer_Output w;

	std::stack<Style> currentStyles;
	Style currentStyle;
	bool outerIsItalic = false;

	std::string currentUrl;
	std::vector<uint32_t> currentOutlineNumbering;

	std::stack<List> currentLists;

	uint32_t remainingSpacesCount = 0;
	bool lastWasWhitespace = true;
	bool lastWasStyleEnd = false;

	char underlineHeadingUsing = 0;
	uint32_t currentHeadingOutlineLevel = 0;
};

class ContentHandler : public ExpatXmlHandler {
public:
	ContentHandler(const Structure &structure, const Styles &styles, ContentContext &context);

public:
	void onStart(const XML_Char *name, const XML_Char **atts) override;
	void onEnd(const XML_Char *name) override;
	void onData(const XML_Char *s, int len) override;

private:
	const Structure &structure;
	const Styles &styles;

	ContentContext &context;
};
