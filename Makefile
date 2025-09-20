# Name of your executable
EXE = h1-counter

# All your source files
SRC = h1-counter.cpp

# C++ flags
CXXFLAGS = -Wall -g -std=c++11

# Any additional required libraries
LDLIBS =

# C++ compiler to use
CXX = g++

.PHONY: all
all: $(EXE)

# Implicit rules defined by Make if you name your source file
# the same as the executable file, but you can redefine if needed

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDLIBS) -o $(EXE)

.PHONY: clean
clean:
	rm -f $(EXE)
