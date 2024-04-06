#pragma once
#define _XOPEN_SOURCE_EXTENDED 1

#include "ncurses.h"

#include <deque>
#include <list>
#include <string>

static constexpr size_t TAB_WIDTH = 4;
static constexpr int A_SPECIAL = A_DIM | A_UNDERLINE;

class Editor {
	private:
		std::string fileName;

		std::list<std::deque<unsigned char>> lines;
		std::list<std::deque<unsigned char>>::iterator rowIter;
		size_t row = 0, col = 0, firstRow = 0, firstCol = 0;
		WINDOW *textWindow = NULL, *statusWindow = NULL;

		inline unsigned char getChar();
		void writeChar(unsigned char c);
		void incrementChar();
		void decrementChar();

		void splitLine();
		void mergeLines(const unsigned char c);

	public:
		Editor(std::string& fileName);
		~Editor();

		void start();

		void printBuffer();
		void printStatus(const std::string& message = std::string());

		void writeBuffer();

	private:
		void handleInput(int input);
		void moveUp();
		void moveDown();
		void moveLeft();
		void moveRight();
};

// vim: sw=2:ts=2:noexpandtab
