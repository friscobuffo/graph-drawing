# Compiler
COMPILER = g++

# Source Files
SRCS = src/main.cpp \
		src/sat/glucose.cpp \
		src/sat/cnf_builder.cpp \
		src/orthogonal/shape.cpp \
		src/orthogonal/shape_builder.cpp \
		src/drawing/polygon.cpp \
		src/drawing/svg_drawer.cpp \
		src/orthogonal/drawing_builder.cpp

# Optimization flags
OPTFLAGS = -O3 -march=native

# C++ standard
CXXFLAGS = -std=c++20 $(OPTFLAGS)

# Object Files (stored in obj/ directory)
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Output directories
OBJ_DIR = obj

# Target output
TARGET = main

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(COMPILER) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Rule to compile .cpp files to .o files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@