# Compiler
COMPILER = g++

# Source Files
SRCS = src/main.cpp \
		src/sat/glucose.cpp \
		src/orthogonal/shape.cpp

# C++ standard
CXXFLAGS = -std=c++20

# Object Files (stored in obj/ directory)
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

# Dependency Files (stored in obj/ directory)
DEPS = $(SRCS:%.cpp=$(OBJ_DIR)/%.d)

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
$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)/%.d
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

# Rule to generate .d files (dependencies)
$(OBJ_DIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) -M $(CXXFLAGS) $< > $@

# Include the dependency files
-include $(DEPS)
