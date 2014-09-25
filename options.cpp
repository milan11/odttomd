#include "options.h"

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
	}
}

Options &options() {
	return currentOptions;
}
