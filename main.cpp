
#include <fstream>
#include <iostream>
#include <boost/scope_exit.hpp>
#include <zip.h>
#include "content.h"
#include "styles.h"

void main_inner(const char *inputFile) {
	int err = 0;
	zip *z = ::zip_open(inputFile, 0, &err);
	if (z == nullptr)
		throw 2;

	BOOST_SCOPE_EXIT(z) {
		::zip_close(z);
	} BOOST_SCOPE_EXIT_END

	Styles styles;
	{
		zip_file *f = zip_fopen(z, "styles.xml", 0);
		if (f == nullptr)
			throw 3;
		BOOST_SCOPE_EXIT(f) {
			zip_fclose(f);
		} BOOST_SCOPE_EXIT_END

		styles = ::parseStyles(f);
	}

	{
		zip_file *f = zip_fopen(z, "content.xml", 0);
		if (f == nullptr)
			throw 3;
		BOOST_SCOPE_EXIT(f) {
			zip_fclose(f);
		} BOOST_SCOPE_EXIT_END

		::parseContent(f, styles);
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "USAGE: odttomd <odt file>" << std::endl;
		return 1;
	}

	try {
		::main_inner(argv[1]);

		return 0;
	} catch (int &result) {
		return result;
	}
}

