#include "options.h"

#include <iostream>

Options currentOptions;

void setProfile(const std::string &name) {
	currentOptions = Options();

	if (name == "standard") {
		currentOptions.headingNumberFormats = true;
		currentOptions.listNumberFormats = false;
		currentOptions.escapeDotInHeadingNumbers = false;
		currentOptions.escapeDotInListNumbers = false;
		currentOptions.escapeDotInText = false;
		currentOptions.bold = true;
		currentOptions.italic = true;
		currentOptions.stylesInHeadingNumbers = true;
		currentOptions.stylesInListNumbers = false;

		return;
	}

	if (name == "github") {
		currentOptions.headingNumberFormats = true;
		currentOptions.listNumberFormats = false;
		currentOptions.escapeDotInHeadingNumbers = false;
		currentOptions.escapeDotInListNumbers = false;
		currentOptions.escapeDotInText = false;
		currentOptions.bold = true;
		currentOptions.italic = true;
		currentOptions.stylesInHeadingNumbers = true;
		currentOptions.stylesInListNumbers = false;

		return;
	}

	if (name == "plain_text") {
		currentOptions.headingNumberFormats = true;
		currentOptions.listNumberFormats = true;
		currentOptions.escapeDotInHeadingNumbers = false;
		currentOptions.escapeDotInListNumbers = false;
		currentOptions.escapeDotInText = false;
		currentOptions.bold = false;
		currentOptions.italic = false;
		currentOptions.stylesInHeadingNumbers = false;
		currentOptions.stylesInListNumbers = false;

		return;
	}

	if (name == "readable") {
		currentOptions.headingNumberFormats = true;
		currentOptions.listNumberFormats = false;
		currentOptions.escapeDotInHeadingNumbers = false;
		currentOptions.escapeDotInListNumbers = false;
		currentOptions.escapeDotInText = false;
		currentOptions.bold = true;
		currentOptions.italic = true;
		currentOptions.stylesInHeadingNumbers = false;
		currentOptions.stylesInListNumbers = false;

		return;
	}

	if (name == "parseable") {
		currentOptions.headingNumberFormats = false;
		currentOptions.listNumberFormats = false;
		currentOptions.escapeDotInHeadingNumbers = false;
		currentOptions.escapeDotInListNumbers = false;
		currentOptions.escapeDotInText = true;
		currentOptions.bold = true;
		currentOptions.italic = true;
		currentOptions.stylesInHeadingNumbers = false;
		currentOptions.stylesInListNumbers = false;

		return;
	}

	std::cerr << "Unknown profile: " << name << std::endl;
	throw 50;
}

Options &options() {
	return currentOptions;
}
