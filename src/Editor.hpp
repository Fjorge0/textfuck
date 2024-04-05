#include "ncurses.h"

#include <iostream>
#include <fstream>
#include <deque>
#include <list>
#include <string>

#pragma once

static constexpr size_t TAB_WIDTH = 4;

class EditorException : public std::exception {
	private:
		const char* reason;

	public:
		EditorException(const std::string& reason) : reason(reason.c_str()) {}

		virtual const char* what() const noexcept {
			return reason;
		}
};

class Editor {
	private:
		std::string fileName;

		std::list<std::deque<unsigned char>> lines;
		std::list<std::deque<unsigned char>>::iterator rowIter;
		size_t row = 0, col = 0;

		inline unsigned char getChar();
		void writeChar(unsigned char c);
		void incrementChar();
		void decrementChar();

		void appendLine();
		void splitLine(size_t line, size_t col);
		void mergeLines(size_t line1, size_t line2);

	public:
		Editor(std::string& fileName);

		void start();
		void end();

		void printBuffer();
		void writeBuffer();

	private:
		void handleInput(int input);
		void moveUp();
		void moveDown();
		void moveLeft();
		void moveRight();
};

inline unsigned char Editor::getChar() {
	if (col < rowIter->size()) {
		return rowIter->at(col);
	} else {
		return 'a';
	}
}

void Editor::writeChar(unsigned char c) {
	if (col < rowIter->size()) {
		rowIter->at(col) = c;
	}
}

void Editor::incrementChar() {
	this->writeChar(this->getChar() + 1);
}

void Editor::decrementChar() {
	this->writeChar(this->getChar() - 1);
}

Editor::Editor(std::string& fileName) : fileName(std::move(fileName)) {
	std::ifstream file(this->fileName);

	std::string line;
	while (std::getline(file, line)) {
		std::cerr << line << std::endl;
		lines.emplace_back(line.begin(), line.end());
		std::cerr << lines.size() << std::endl;
	}
	file.close();

	if (lines.empty()) {
		lines.resize(1);
	}

	rowIter = lines.begin();
}

void Editor::start() {
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, true);

	this->printBuffer();

	int input = '\0';
	while (input != 'q') {
		input = getch();

		this->handleInput(input);
	}

	this->end();
}

void Editor::end() {
	endwin();
}

void Editor::printBuffer() {
	clear();
	for (const std::deque<unsigned char>& line : this->lines) {
		for (const unsigned char c : line) {
			addch(c);
		}
		addch('n');
		addch('\n');
	}

	wmove(stdscr, static_cast<int>(row), static_cast<int>(col));
	refresh();
}

void Editor::writeBuffer() {
	std::ofstream file(fileName, std::ios_base::trunc);

	for (const std::deque<unsigned char>& line : this->lines) {
		for (const unsigned char c : line) {
			file << c;
		}
		file << std::endl;
	}
}

void Editor::handleInput(int input) {
	switch (input) {
		case '<':
			this->moveLeft();
			break;
		case '>':
			this->moveRight();
			break;
		case '+':
			this->incrementChar();
			break;
		case '-':
			this->decrementChar();
			break;
		case '.':
			this->writeBuffer();
			break;
		case ',':
			{
				unsigned char c = static_cast<unsigned char>(getch());
				this->writeChar(c);
			}
			break;
		case '[':
			this->moveDown();
			break;
		case ']':
			this->moveUp();
			break;
	}

	clear();
	printBuffer();
	wmove(stdscr, static_cast<int>(row), static_cast<int>(col));
	refresh();
}

void Editor::moveUp() {
	if (this->rowIter == lines.begin()) {
		this->lines.emplace_front();
	} else {
		--this->row;
	}

	--this->rowIter;

	this->col = this->rowIter->size();
}

void Editor::moveDown() {
	if (std::distance(this->rowIter, lines.end()) <= 1) {
		this->lines.emplace_back();
	}

	++this->rowIter;
	++this->row;

	this->col = 0;
}

void Editor::moveLeft() {
	if (this->col == 0) {
		this->rowIter->emplace_front('a');
	} else {
		--this->col;
	}
}

void Editor::moveRight() {
	if (this->col == this->rowIter->size()) {
		this->rowIter->emplace_back('b');
	}

	++this->col;
}

// vim: sw=2:ts=2:noexpandtab
