#include "Editor.hpp"

#include <getopt.h>
#include <iostream>

int main(int argc, char** argv) {
	constexpr option longopts[] {
		{ "help", no_argument, nullptr, 'h' },
		{ nullptr, no_argument, nullptr, '\0' }
	};

	int opt;
	while ((opt = getopt_long(argc, argv, ":h", longopts, nullptr)) >= 0) {
		switch (opt) {
			case 'h':
				std::cout << "Usage: " << argv[0] << " file [options]\n"
									<< '\n'
									<< "Options:\n"
									<< " -h\t--help\tPrints this message and exits."
									<< std::endl;
				return 0;
				break;
			case '?':
				std::cerr << "Error: Unknown option " << static_cast<char>(optopt) << std::endl;
				return 1;
				break;
		}
	}

	// Set the file name
	std::string fileName;
	if (optind >= argc) {
		std::cerr << "Error: No file specified." << std::endl;
		return 1;
	} else {
		fileName = argv[optind];
	}

	try {
		Editor editor(fileName);
		editor.start();
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 2;
	}

	// Exit
	return 0;
}

// vim: ts=2:sw=2:noexpandtab
