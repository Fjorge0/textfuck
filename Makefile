TARGET = "src/main.cpp"
EXECUTABLE = textfuck

CXX = g++
CXXFLAGS = -std=c++23 -Wconversion -Wall -Werror -Wextra -pedantic

release:
	$(CXX) $(CXXFLAGS) -O3 $(TARGET) -o $(EXECUTABLE)
.PHONY: release

debug:
	$(CXX) $(CXXFLAGS) -Og -g3 -fsanitize=address -fsanitize=undefined $(TARGET) -o $(EXECUTABLE)_debug
.PHONY: debug

clean:
	rm -fv $(EXECUTABLE)*
.PHONY: clean
