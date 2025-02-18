# Compiler
COMPILER = g++

# Source Files
SRCS = src/main.cpp \
		src/sat/glucose.cpp \
		src/orthogonal/shape.cpp \
		src/drawing/polygon.cpp \
		src/drawing/svg_drawer.cpp

# C++ standard
CXXFLAGS = -std=c++20

# Object Files (stored in obj/ directory)
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Output directories
OBJ_DIR = bin

# Target output
TARGET = main

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(COMPILER) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Optimization flags
OPTFLAGS = -O3

# Rule to compile .cpp files to .o files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) $(INCLUDES) -c $< -o $@