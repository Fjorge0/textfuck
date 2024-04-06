SOURCEDIR = src
TARGET = $(SOURCEDIR)/main.cpp
OBJECTS = Editor.obj
EXECUTABLE = textfuck

CXX = g++
CXXFLAGS = -std=c++23 -Wconversion -Wall -Werror -Wextra -pedantic
LIBS = -lncurses

all: CXXFLAGS += -O3
all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(TARGET) -o $(EXECUTABLE) $(OBJECTS) $(LIBS)
.PHONY: all

all_debug: CXXFLAGS += -Og -g3 -fsanitize=address -fsanitize=undefined
all_debug: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(TARGET) -o $(EXECUTABLE) $(OBJECTS) $(LIBS)
.PHONY: all_debug

clean:
	rm -fv $(EXECUTABLE)* *.obj
.PHONY: clean

$(EXECUTABLE):
	$(CXX) $(CXXFLAGS) $(TARGET) -o $(EXECUTABLE) $(OBJECTS) $(LIBS)

$(OBJECTS): %.obj:
	$(CXX) $(CXXFLAGS) -c $(SOURCEDIR)/$<.cpp -o $@ $(LIBS)

