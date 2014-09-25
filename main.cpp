
#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>
#include <zip.h>
#include "content.h"
#include "options.h"
#include "styles.h"

void main_inner(const char *inputFile) {
	int err = 0;
	zip *z = ::zip_open(inputFile, 0, &err);
	if (z == nullptr)
		throw 2;

	BOOST_SCOPE_EXIT(z) {
		::zip_close(z);
	} BOOST_SCOPE_EXIT_END

	StylesContext stylesContext;
	{
		zip_file *f = zip_fopen(z, "styles.xml", 0);
		if (f == nullptr)
			throw 3;
		BOOST_SCOPE_EXIT(f) {
			zip_fclose(f);
		} BOOST_SCOPE_EXIT_END

		stylesContext = ::parseStyles(f);
	}

	{
		zip_file *f = zip_fopen(z, "content.xml", 0);
		if (f == nullptr)
			throw 4;
		BOOST_SCOPE_EXIT(f) {
			zip_fclose(f);
		} BOOST_SCOPE_EXIT_END

		::parseContent(f, stylesContext);
	}
}

int main(int argc, char *argv[]) {
	namespace po = boost::program_options;

	po::options_description desc("Usage");
	desc.add_options()
		("help", "shows usage")
		("odt_file", po::value<std::string>(), "input ODT (OpenDocument Text) file")
		("profile", po::value<std::string>()->default_value("standard"), "output profile")
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
		return 1;
	}

	if (vm.count("profile")) {
		::setProfile(vm["profile"].as<std::string>());
	}

	try {
		::main_inner(vm["odt_file"].as<std::string>().c_str());

		return 0;
	} catch (int &result) {
		std::cerr << "ERROR: " << result << std::endl;
		return result;
	}
}

