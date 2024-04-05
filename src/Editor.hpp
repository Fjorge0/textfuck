#include "ncurses.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <deque>
#include <list>
#include <limits>
#include <string>

#pragma once

#define _XOPEN_SOURCE_EXTENDED 1

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
		size_t row = 0, col = 0, rowOffset = 0;
		WINDOW *textWindow = NULL, *statusWindow = NULL;

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
		void printStatus(const std::string& message = std::string());

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
		return '\n';
	}
}

void Editor::writeChar(unsigned char c) {
	if (col < rowIter->size()) {
		rowIter->at(col) = c;
	}
}

void Editor::incrementChar() {
	if (this->getChar() == std::numeric_limits<unsigned char>::max()) {
		this->rowIter->erase(this->rowIter->begin() + this->col);
	} else {
		this->writeChar(this->getChar() + 1);
	}
}

void Editor::decrementChar() {
	if (this->getChar() == '\0') {
		this->rowIter->erase(this->rowIter->begin() + this->col);
	} else {
		this->writeChar(this->getChar() - 1);
	}
}

Editor::Editor(std::string& fileName) : fileName(std::move(fileName)) {
	std::ifstream file(this->fileName);

	std::string line;
	while (std::getline(file, line)) {
		lines.emplace_back(line.begin(), line.end());
	}
	file.close();

	if (lines.empty()) {
		lines.resize(1);
	}

	rowIter = lines.begin();
}

void Editor::start() {
	initscr();
	start_color();
	noecho();
	cbreak();
	keypad(stdscr, true);

	int x, y;
	getmaxyx(stdscr, y, x);
	this->textWindow = subwin(stdscr, y - 6, x - 4, 2, 2);
	this->statusWindow = subwin(stdscr, 1, x - 4, y - 2, 2);

	leaveok(statusWindow, TRUE);
	leaveok(textWindow, FALSE);

	box(stdscr, 0, 0);
	mvhline(y - 3, 0, ACS_HLINE, x);
	mvaddch(y - 3, 0, ACS_LTEE);
	mvaddch(y - 3, x - 1, ACS_RTEE);

	mvwaddstr(stdscr, 0, 1, "textfuck");
	mvwaddstr(stdscr, y - 3, 1, fileName.c_str());

	printBuffer();
	printStatus();

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
	wclear(textWindow);

	for (const std::deque<unsigned char>& line : this->lines) {
		for (const unsigned char c : line) {
			if (std::isprint(c)) {
				waddch(textWindow, c);
			} else {
				waddch(textWindow, c | A_DIM | A_UNDERLINE);
			}
		}

		waddch(textWindow, '^' | A_DIM | A_UNDERLINE);
		waddch(textWindow, 'n' | A_DIM | A_UNDERLINE);
		waddch(textWindow, '\n');
	}

	wmove(textWindow, static_cast<int>(row), static_cast<int>(col));

	int y, x, parY, parX;
	getyx(textWindow, y, x);
	getparyx(textWindow, parY, parX);
	move(y + parY, x + parX);

	touchwin(stdscr);
	refresh();
}

void Editor::printStatus(const std::string& message) {
	wclear(statusWindow);

	wprintw(statusWindow, "%03u\t Row %*zu/%zu\tCol %*zu/%zu", this->getChar(), static_cast<int>(std::log10(lines.size())), row + 1, lines.size(), static_cast<int>(std::log10(rowIter->size() + 1)), col + 1, rowIter->size() + 1);

	std::size_t x = static_cast<std::size_t>(getmaxx(statusWindow));
	mvwaddstr(statusWindow, 0, static_cast<int>(x - message.size()), message.c_str());

	touchwin(stdscr);
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

	file.close();
}

void Editor::handleInput(int input) {
	std::string message;

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
			message = "File written";
			break;
		case ',':
			{
				printStatus("Replace");
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

	printStatus(message);
	printBuffer();
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
		this->rowIter->emplace_back('a');
	}

	++this->col;
}

// vim: sw=2:ts=2:noexpandtab
