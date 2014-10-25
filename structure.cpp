#include "structure.h"

#include <cstring>
#include <iostream>
#include "expat_utils.h"

void Structure::appendText(const std::string &bookmarkName, const std::string &text)
{
	for (NameToText::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
	{
		if (it->first == bookmarkName)
		{
			it->second += text;
			return;
		}
	}

	throw "Invalid bookmark name: " + bookmarkName;
}

std::string Structure::findText(const std::string &bookmarkName) const
{
	NameToText::const_iterator it_found = bookmarks.end();
	for (NameToText::const_iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
	{
		if (it->first == bookmarkName)
		{
			it_found = it;
			break;
		}
	}

	if (it_found == bookmarks.end())
	{
		std::cerr << "Bookmark not found: " << bookmarkName << std::endl;
		return "";
	}

	uint32_t equalTextBeforeCount = 0;
	for (NameToText::const_iterator it = bookmarks.begin(); it != it_found; ++it)
	{
		if (it->second == it_found->second)
		{
			++equalTextBeforeCount;
		}
	}

	std::string bookmarkText = it_found->second;
	if (equalTextBeforeCount > 0)
	{
		bookmarkText += '-' + std::to_string(equalTextBeforeCount);
	}

	return bookmarkText;
}

StructureHandler::StructureHandler(StructureContext &context)
	: context(context)
{
}

void StructureHandler::onStart(const XML_Char *name, const XML_Char **atts) {
	bool bookmarkStart = (! ::strcmp(name, "text:bookmark-start"));
	bool bookmarkEnd = (! ::strcmp(name, "text:bookmark-end"));

	if (bookmarkStart || bookmarkEnd) {
		const std::string bookmarkName = ::attrString(atts, "text:name", "");
		if (! bookmarkName.empty()) {
			if (bookmarkStart) {
				if (!context.textIsFromPreviousH) flushCollectedTextToBookmarks();
				context.currentBookmarkNames.insert(bookmarkName);
				context.structure.bookmarks.push_back(std::make_pair(bookmarkName, ""));
				if (context.textIsFromPreviousH) flushCollectedTextToBookmarks();
			}
			if (bookmarkEnd) {
				flushCollectedTextToBookmarks();
				context.currentBookmarkNames.erase(bookmarkName);
			}
		} else {
			std::cerr << "Empty bookmark name" << std::endl;
		}
	}
	else if (! ::strcmp(name, "text:h")) {
		flushCollectedTextToBookmarks();
		context.textIsFromPreviousH = true;
	}
	else
	{
		context.textIsFromPreviousH = false;
	}
}

void StructureHandler::onEnd(const XML_Char *) {
	context.textIsFromPreviousH = false;
}

void StructureHandler::onData(const XML_Char *, int) {

}

void StructureHandler::flushCollectedTextToBookmarks()
{
	const std::string collectedText = context.visibleTextCollecting.getCollectedVisibleText();

	for (const std::string &bookmarkName : context.currentBookmarkNames) {
		context.structure.appendText(bookmarkName, collectedText);
	}

	context.visibleTextCollecting.resetCollectedVisibleText();

	context.textIsFromPreviousH = false;
}
