#include "structure.h"

#include <cstring>
#include <iostream>
#include "expat_utils.h"

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
				context.currentBookmarkNames.insert(bookmarkName);
			}
			if (bookmarkEnd) {
				context.currentBookmarkNames.erase(bookmarkName);
			}
		} else {
			std::cerr << "Empty bookmark name" << std::endl;
		}
	}
}

void StructureHandler::onEnd(const XML_Char *name) {

}

void StructureHandler::onData(const XML_Char *s, int len) {
	for (const std::string &bookmarkName : context.currentBookmarkNames) {
		context.structure.bookmarks[bookmarkName] += std::string(s, len);
	}
}
