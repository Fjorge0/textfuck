SOURCEDIR = src
TARGET = $(SOURCEDIR)/main.cpp
EXECUTABLE = textfuck

CXX = g++
CXXFLAGS = -std=c++23 -Wconversion -Wall -Werror -Wextra -pedantic -lncurses

release: CXXFLAGS += -O3
release: Editor.obj
	$(CXX) $(CXXFLAGS) $(TARGET) -o $(EXECUTABLE) Editor.obj
.PHONY: release

debug: CXXFLAGS += -Og -g3 -fsanitize=address -fsanitize=undefined
debug: Editor.obj
	$(CXX) $(CXXFLAGS) $(TARGET) -o $(EXECUTABLE)_debug Editor.obj
.PHONY: debug

clean:
	rm -fv $(EXECUTABLE)* *.obj
.PHONY: clean

Editor.obj:
	$(CXX) $(CXXFLAGS) -c $(SOURCEDIR)/Editor.cpp -o Editor.obj
