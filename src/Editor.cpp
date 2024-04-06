#include "Editor.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <limits>

inline unsigned char Editor::getChar() {
	if (col < rowIter->size()) {
		return rowIter->at(col);
	} else {
		return '\n';
	}
}

void Editor::writeChar(unsigned char c) {
	if (c == '\n') {
		splitLine();
	} else if (col < rowIter->size()) {
		rowIter->at(col) = c;
	} else {
		mergeLines(c);
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

void Editor::splitLine() {
	auto beforeIter = this->lines.emplace(this->rowIter);
	beforeIter->insert(beforeIter->end(), this->rowIter->begin(), this->rowIter->begin() + col);
	this->rowIter->erase(this->rowIter->begin(), this->rowIter->begin() + col + 1);

	this->rowIter = beforeIter;
}

void Editor::mergeLines(const unsigned char c) {
	auto beforeIter = this->rowIter++;
	beforeIter->push_back(c);
	if (this->rowIter != this->lines.end()) {
		beforeIter->insert(beforeIter->end(), this->rowIter->begin(), this->rowIter->end());
		this->lines.erase(this->rowIter);

		this->rowIter = beforeIter;
	} else {
		--this->rowIter;
	}
}

Editor::Editor(std::string& fileName) : fileName(std::move(fileName)) {
	if (std::filesystem::exists(std::filesystem::path(this->fileName))) {
		std::ifstream file(this->fileName);
		if (!file.is_open()) {
			throw std::runtime_error("Error: Cannot read from file " + this->fileName);
		}

		std::string line;
		while (std::getline(file, line)) {
			lines.emplace_back(line.begin(), line.end());
		}
		file.close();
	}

	if (lines.empty()) {
		lines.resize(1);
	}

	rowIter = lines.begin();
}

Editor::~Editor() {
	endwin();
}

void Editor::start() {
	// Initialize ncurses
	initscr();
	start_color();
	noecho();
	cbreak();
	keypad(stdscr, true);

	// Create windows
	int x, y;
	getmaxyx(stdscr, y, x);
	this->textWindow = subwin(stdscr, y - 6, x - 4, 2, 2);
	this->statusWindow = subwin(stdscr, 1, x - 4, y - 2, 2);

	// Draw the borders around the windows
	box(stdscr, 0, 0);
	mvhline(y - 3, 0, ACS_HLINE, x);
	mvaddch(y - 3, 0, ACS_LTEE);
	mvaddch(y - 3, x - 1, ACS_RTEE);

	// Print box titles
	mvwaddstr(stdscr, 0, 1, "textfuck");
	mvwaddstr(stdscr, y - 3, 1, fileName.c_str());

	// Print initial window
	printBuffer();
	printStatus();

	// Handle inputs
	int input = '\0';
	while (input != 'q') {
		input = getch();

		this->handleInput(input);
	}
}

void Editor::printBuffer() {
	wclear(textWindow);

	// Get window properties
	int parY, parX, maxY, maxX;
	getmaxyx(textWindow, maxY, maxX);
	getparyx(textWindow, parY, parX);

	// Make variables to store cursor position
	int curX = -1, curY = -1;

	// Check bounds of firstRow for scrolling
	if (this->firstRow > this->row) {
		this->firstRow = this->row;
	} else if (static_cast<int>(this->row - this->firstRow) >= maxY) {
		this->firstRow = this->row - maxY + 1;
	}

	// Check bounds of firstCol for scrolling
	if (this->firstCol > this->col) {
		this->firstCol = this->col;
	} else if (static_cast<int>(this->col - this->firstCol) >= maxX) {
		this->firstCol = this->col - maxX + 1;
	}

	// Draw arrows if vertical scrolling is happening
	mvwhline(stdscr, 1, 2, (this->firstRow > 0 ? ACS_UARROW | A_REVERSE : ' '), 3);
	mvwhline(stdscr, parY + maxY, 2, (static_cast<int>(this->lines.size() - this->firstRow) > maxY ? ACS_DARROW | A_REVERSE : ' '), 3);

	// Lambda to print rows
	const auto printLine = [this, &parX, &parY, &maxX, &curY, &curX] (const std::deque<unsigned char>& line, const size_t row) {
		// Get current cursor position
		int y, x;
		getyx(textWindow, y, x);
		int firstY = y;

		// If this is the correct row, make the cursor move here later
		if (row == this->row) {
			curY = y;
		}

		// Print each column
		size_t col = this->firstCol;
		while (x < maxX && (y == firstY || col < this->col)) {
			if (col >= line.size()) {
				break;
			}

			if (row == this->row && col == this->col && y == firstY) {
				curX = x;
			}

			// Switch with some special characters that aren't printed
			switch(line[col]) {
				case 8:
					waddch(textWindow, '\\' | A_SPECIAL);
					waddch(textWindow, 'b' | A_SPECIAL);
					break;
				case 13:
					waddch(textWindow, '\\' | A_SPECIAL);
					waddch(textWindow, 'r' | A_SPECIAL);
					break;
				case '\t': {
						waddch(textWindow, '\\' | A_SPECIAL);
						waddch(textWindow, 't' | A_SPECIAL);
						for (size_t i = 1; i < TAB_WIDTH; ++i) {
							waddch(textWindow, ' ');
						}
					}
					break;
				default:
					waddch(textWindow, line[col] | (std::isprint(line[col]) ? 0 : A_SPECIAL));
			}
			++col;

			// Update cursor position
			getyx(textWindow, y, x);
		}

		// Print a newline character
		if (line.size() == this->firstCol || x > 0) {
			if (this->col == line.size() && y == firstY) {
				curX = x;
			}
			waddch(textWindow, '\\' | A_SPECIAL);
		}
		getyx(textWindow, y, x);

		// Account for multiline characters
		if (row == this->row && curX < 0 && y != firstY) {
			int adjustment = x + maxX * (y - firstY - 1);
			this->firstCol += (adjustment == 0 ? 1 : adjustment);
		}

		// Clear future lines if overflow exists
		getyx(textWindow, y, x);
		if (y != firstY) {
			wmove(textWindow, firstY + 1, 0);
			wclrtobot(textWindow);
		}

		// Print an arrow if there is horizontal scrolling
		mvwaddch(stdscr, parY + firstY, 1, (this->firstCol > 0 && line.size() > 0 ? ACS_LARROW | A_REVERSE : ' '));
		if (line.size() > 0 && this->firstCol > 0 && x == 0 && y == firstY) {
			wattron(textWindow, A_REVERSE);
			wprintw(textWindow, " %lu", this->firstCol - line.size());
			wattroff(textWindow, A_REVERSE);
		}
		mvwaddch(stdscr, parY + firstY, parX + maxX, (col < line.size() ? ACS_RARROW | A_REVERSE : ' '));
	};

	// Print the current row to let it calculate column
	printLine(*this->rowIter, this->row);
	wclear(textWindow);

	// Print rows above the current row
	auto iter = this->rowIter;
	for (size_t row = this->row; row > this->firstRow && row < this->lines.size(); --row) {
		--iter;
	}
	for (size_t row = 0; row < static_cast<size_t>(maxY) && iter != lines.end(); ++row) {
		wmove(textWindow, static_cast<int>(row), 0);
		printLine(*iter, this->firstRow + row);
		++iter;
	}

	// Move cursor to currently selected character
	wmove(textWindow, curY, curX);
	move(parY + curY, parX + curX);

	// Redraw the window
	touchwin(stdscr);
	refresh();
}

void Editor::printStatus(const std::string& message) {
	wclear(statusWindow);

	// Print current character value, row, and column
	wprintw(statusWindow, "%03u\t Row %*zu/%zu\tCol %*zu/%zu", this->getChar(), static_cast<int>(std::ceil(std::log10(lines.size() + 1))), row + 1, lines.size(), static_cast<int>(std::ceil(std::log10(rowIter->size() + 2))), col + 1, rowIter->size() + 1);

	// Print custom status on right side
	std::size_t x = static_cast<std::size_t>(getmaxx(statusWindow));
	mvwaddstr(statusWindow, 0, static_cast<int>(x - message.size()), message.c_str());

	// Redraw the window
	touchwin(stdscr);
	refresh();
}

void Editor::writeBuffer() {
	std::ofstream file(fileName, std::ios_base::trunc);
	if (!file.is_open()) {
		printStatus("Cannot write to file");
	} else {
		for (const std::deque<unsigned char>& line : this->lines) {
			for (const unsigned char c : line) {
				file << c;
			}
			file << std::endl;
		}

		file.close();
		printStatus("File written");
	}
}

void Editor::handleInput(int input) {
	printStatus("Normal");
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
				printStatus("Replace");
				unsigned char c = static_cast<unsigned char>(getch());
				this->writeChar(c);
				printStatus("Normal");
			}
			break;
		case '[':
			this->moveDown();
			break;
		case ']':
			this->moveUp();
			break;
	}

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
