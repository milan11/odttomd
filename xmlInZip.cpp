#include "xmlInZip.h"

#include <boost/scope_exit.hpp>

namespace {
	void onStart(void *userData, const XML_Char *name, const XML_Char **atts) {
		const Handlers *handlers = static_cast<const Handlers *>(userData);

		for (const std::shared_ptr<ExpatXmlHandler> &handler : *handlers) {
			handler->onStart(name, atts);
		}
	}

	void onEnd(void *userData, const XML_Char *name) {
		const Handlers *handlers = static_cast<const Handlers *>(userData);

		for (const std::shared_ptr<ExpatXmlHandler> &handler : *handlers) {
			handler->onEnd(name);
		}
	}

	void onData(void *userData, const XML_Char *s, int len) {
		const Handlers *handlers = static_cast<const Handlers *>(userData);

		for (const std::shared_ptr<ExpatXmlHandler> &handler : *handlers) {
			handler->onData(s, len);
		}
	}
}

void processXmlInZip(zip * const z, const std::string &xmlFileName, Handlers &handlers) {
	XML_Parser parser = ::XML_ParserCreate(nullptr);
	BOOST_SCOPE_EXIT(&parser) {
		::XML_ParserFree(parser);
	} BOOST_SCOPE_EXIT_END

	::XML_SetUserData(parser, &handlers);
	::XML_SetElementHandler(parser, &onStart, &onEnd);
	::XML_SetCharacterDataHandler(parser, &onData);

	zip_file *f = zip_fopen(z, xmlFileName.c_str(), 0);
	if (f == nullptr)
		throw "Unable to open XML file: " + xmlFileName;
	BOOST_SCOPE_EXIT(f) {
		zip_fclose(f);
	} BOOST_SCOPE_EXIT_END

	static const size_t bufferSize = 4 * 1024;
	char buffer[bufferSize];

	int64_t readResult = 0;

	while ((readResult = zip_fread(f, buffer, bufferSize)) > 0) {
		if (::XML_Parse(parser, buffer, static_cast<int>(readResult), false) == 0)
			throw "Unable to parse XML file: " + xmlFileName;
	}

	if (readResult < 0) {
		throw "Unable to read data from XML file: " + xmlFileName;
	}

	if (::XML_Parse(parser, buffer, 0, true) == 0)
		throw "Unable to parse end of XML file: " + xmlFileName;
}
