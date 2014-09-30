#pragma once

#include <memory>
#include <vector>
#include <expat.h>
#include <zip.h>

class ExpatXmlHandler {

public:
	virtual ~ExpatXmlHandler() {};

public:
	virtual void onStart(const XML_Char *, const XML_Char **) {}
	virtual void onEnd(const XML_Char *) {}
	virtual void onData(const XML_Char *, int) {}
};

using Handlers = std::vector<std::shared_ptr<ExpatXmlHandler>>;

void processXmlInZip(zip * const z, const std::string &xmlFileName, Handlers &handlers);
