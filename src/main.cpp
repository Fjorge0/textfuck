#include "Editor.hpp"
#include <iostream>

int main(int argc, char** argv) {
	// Set the file name
	std::string fileName;
	if (argc == 1) {
		std::cerr << "No file specified" << std::endl;
		return 1;
	} else {
		fileName = argv[1];
	}

	Editor editor(fileName);
	editor.start();

	// Exit
	return 0;
}

// vim: ts=2:sw=2:noexpandtab
