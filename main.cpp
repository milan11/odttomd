#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>
#include <zip.h>
#include "content.h"
#include "options.h"
#include "styles.h"
#include "xmlInZip.h"

void main_inner(const std::string &inputFile, const std::string &profile) {
	::setProfile(profile);

	int err = 0;
	zip *z = ::zip_open(inputFile.c_str(), 0, &err);
	if (z == nullptr)
		throw "Unable to read input file: " + inputFile;

	BOOST_SCOPE_EXIT(z) {
		::zip_close(z);
	} BOOST_SCOPE_EXIT_END

	StylesContext stylesContext;
	{
		Handlers handlers;
		handlers.push_back(std::make_shared<StylesHandler>(stylesContext));
		::processXmlInZip(z, "styles.xml", handlers);
	}

	ContentContext contentContext;
	{
		Handlers handlers;
		handlers.push_back(std::make_shared<ContentHandler>(contentContext, stylesContext.styles));
		handlers.push_back(std::make_shared<StylesHandler>(stylesContext));
		::processXmlInZip(z, "content.xml", handlers);
	}
}

struct ProfileOption {
	ProfileOption() {}

	explicit ProfileOption(std::string value)
		: value(std::move(value))
	{
	}

	std::string value;
};

std::ostream & operator<<(std::ostream &os, const ProfileOption &opt) {
	os << opt.value;
	return os;
}

void validate(boost::any &v, std::vector<std::string> const& values, ProfileOption *, int) {
	using namespace boost::program_options;

		validators::check_first_occurrence(v);

		const std::string &s = validators::get_single_string(values);
		const std::vector<std::string> availableProfiles = ::getAvailableProfiles();
		if (std::find(availableProfiles.begin(), availableProfiles.end(), s) != availableProfiles.end()) {
			v = boost::any(ProfileOption(s));
		} else {
			throw validation_error(validation_error::invalid_option_value);
		}
}

int main(int argc, char *argv[]) {
	namespace po = boost::program_options;

	::initProfiles();

	po::options_description desc("Usage");
	desc.add_options()
		("help", "shows usage")
		("odt_file", po::value<std::string>()->required(), "input ODT (OpenDocument Text) file")
		("profile", po::value<ProfileOption>()->default_value(ProfileOption("standard")), ("output type: " + boost::algorithm::join(::getAvailableProfiles(), ", ")).c_str())
	;

	po::positional_options_description p;
	p.add("odt_file", 1);

	po::variables_map vm;

	bool showHelp = false;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);
	} catch (const boost::program_options::error &e) {
		std::cerr << e.what() << std::endl;
		showHelp = true;
	}

	if (showHelp) {
		std::cout << desc << std::endl;
		return 2;
	}

	try {
		::main_inner(vm["odt_file"].as<std::string>(), vm["profile"].as<ProfileOption>().value);

		return 0;
	} catch (const std::string &str) {
		std::cerr << "[ERROR] " << str << std::endl;
		return 1;
	}
}
