CXX = g++
CXXFLAGS = -std=c++11 -Wall -I./include

# Specify source directory
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(SOURCES:$(SRCDIR)/%.cpp=%.o)
EXEC = 434fe

$(EXEC): $(OBJ)
	$(CXX) -o $(EXEC) $(OBJ)

%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: clean