CXX = g++
CXXFLAGS = -std=c++20 -Wall -I./include -O3 -march=native -flto

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