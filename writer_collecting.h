#pragma once

#include <sstream>
#include "writer.h"

class Writer_Collecting : public Writer {

public:
	void resetCollectedVisibleText();
	std::string getCollectedVisibleText() const;

public:
	virtual void writeMarkup(const char c) override;
	virtual void writeMarkup(const std::string &str) override;

	virtual void writeVisibleText(const char c) override;
	virtual void writeVisibleText(const std::string &str) override;

	virtual void writeVisibleText_escaped(const char c) override;
	virtual void writeVisibleText_escaped(const std::string &str) override;

private:
	void write(const char c);
	void write(const std::string &str);

private:
	std::ostringstream collectedVisibleText;

};
