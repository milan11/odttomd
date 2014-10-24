#pragma once

#include <iosfwd>
#include "writer.h"

class Writer_Output : Writer {

public:
	Writer_Output(std::ostream &os);

	void resetCodePointsCount();
	uint32_t getCodePointsCount() const;

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
	std::ostream &os;
	uint32_t codePointsCount = 0;

};
