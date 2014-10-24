#pragma once

#include <sstream>
#include <string>
#include "writer.h"

class Writer_Collecting : public Writer {

public:
	void resetCollectedVisibleText();
	std::string getCollectedVisibleText() const;

public:
	virtual void writeMarkup(const char c) override;
	virtual void writeVisibleText(const char c) override;
	virtual void writeVisibleText_escaped(const char c) override;

private:
	void write(const char c);

private:
	std::ostringstream collectedVisibleText;

};
