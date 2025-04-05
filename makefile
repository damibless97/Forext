# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

# Source directory
SRC_DIR = src/forext

# Source files (all .cpp files in the src/forextvm directory)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files (replace .cpp with .o)
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = forext

# Default target
all: $(TARGET)

# Link all object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile each .cpp file into an object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean