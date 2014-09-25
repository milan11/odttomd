#include "options.h"

Options currentOptions;

void setProfile(const std::string &name) {
	currentOptions = Options();

	if (name == "standard") {
		currentOptions.supportBold = true;
		currentOptions.supportItalic = true;
	}

	if (name == "github") {
		currentOptions.supportBold = true;
		currentOptions.supportItalic = true;
	}

	if (name == "plain") {
		currentOptions.supportBold = false;
		currentOptions.supportItalic = false;
	}

	if (name == "readable") {
		currentOptions.supportBold = true;
		currentOptions.supportItalic = true;
	}

	if (name == "strict") {
		currentOptions.supportBold = true;
		currentOptions.supportItalic = true;
	}
}

Options &options() {
	return currentOptions;
}
