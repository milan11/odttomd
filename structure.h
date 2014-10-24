#pragma once

#include <map>
#include <string>
#include <set>
#include "writer_collecting.h"
#include "xmlInZip.h"

class Structure {

public:
	using NameToText = std::map<std::string, std::string>;
	NameToText bookmarks;

};

class StructureContext {

public:
	Structure structure;

	std::set<std::string> currentBookmarkNames;
	Writer_Collecting visibleTextCollecting;
};

class StructureHandler : public ExpatXmlHandler {
public:
	StructureHandler(StructureContext &context);

public:
	void onStart(const XML_Char *name, const XML_Char **atts) override;
	void onEnd(const XML_Char *name) override;
	void onData(const XML_Char *s, int len) override;

private:
	StructureContext &context;
};
