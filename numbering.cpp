#include "numbering.h"

#include <algorithm>
#include <iostream>
#include <map>

namespace numbering {

std::string createLetters(const uint32_t number, const char base) {
	uint32_t currentNumber = number;
	std::string result;
	while (currentNumber > 0) {
		result.push_back(static_cast<char>(static_cast<uint32_t>(base) + ((currentNumber - 1) % 26)));
		--currentNumber;
		currentNumber /= 26;
	}
	std::reverse(result.begin(), result.end());
	return result;
}

std::string createLettersSync(const uint32_t number, const char base) {
	return std::string(((number - 1) / 26) + 1, static_cast<uint32_t>(base) + (number - 1) % 26);
}

std::string createRoman(const uint32_t number, const bool upperCase) {
	auto fixCase = [upperCase](const char c) -> std::string {
		if (upperCase)
			return std::string(1, c);
		else
			return std::string(1, ::tolower(c));
	};

	using ConversionRules = std::map<uint16_t, std::string>;
	ConversionRules conversionRules;
	conversionRules[1] = fixCase('I');
	conversionRules[5] = fixCase('V');
	conversionRules[10] = fixCase('X');
	conversionRules[50] = fixCase('L');
	conversionRules[100] = fixCase('C');
	conversionRules[500] = fixCase('D');
	conversionRules[1000] = fixCase('M');

	auto addSpecial = [&conversionRules](const uint16_t num, const uint16_t sub) {
		conversionRules[num - sub] = conversionRules[sub] + conversionRules[num];
	};

	addSpecial(5, 1);
	addSpecial(10, 1);
	addSpecial(50, 10);
	addSpecial(100, 10);
	addSpecial(500, 100);
	addSpecial(1000, 100);

	uint32_t currentNumber = number;
	std::string result;
	for (ConversionRules::const_reverse_iterator it = conversionRules.rbegin(); (it != conversionRules.rend()) && (currentNumber > 0); ++it) {
		while (currentNumber >= it->first) {
			result += it->second;
			currentNumber -= it->first;
		}
	}

	return result;
}

std::string createNumber(const uint32_t number, const char format, const bool letterSync) {
	if (number == 0)
		throw "invalid number: " + std::to_string(number);

	if (letterSync) {
		switch (format) {
			case 'A':
				return createLettersSync(number, 'A');
			case 'a':
				return createLettersSync(number, 'a');
		}
	} else {
		switch (format) {
			case '1':
				return std::to_string(number);
			case 'A':
				return createLetters(number, 'A');
			case 'a':
				return createLetters(number, 'a');
			case 'I':
				return createRoman(number, true);
			case 'i':
				return createRoman(number, false);
		}
	}

	std::cerr << "Unsupported numbering format: " << format << " letter sync: " << letterSync << std::endl;

	return "";
}

}
