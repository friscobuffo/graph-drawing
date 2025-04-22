COMPILER := g++
OBJ_DIR := obj

# Directory flags
OGDF_INCLUDES := -IOGDF/include
CXXFLAGS := -std=c++20 -O3 -march=native

# OGDF flags (only for files that need it)
OGDF_LDFLAGS := -LOGDF -lOGDF -lCOIN

# Source files
COMMON_SRCS := src/sat/glucose.cpp \
    src/sat/cnf_builder.cpp \
    src/orthogonal/shape.cpp \
    src/orthogonal/shape_builder.cpp \
    src/drawing/polygon.cpp \
    src/drawing/svg_drawer.cpp \
    src/orthogonal/drawing_builder.cpp \
    src/config/config.cpp \
    src/core/graph/generators.cpp

# Files with main() functions
STATS_MAIN_SRC := src/stats.cpp
GEN_MAIN_SRC := src/gen.cpp
MAIN_MAIN_SRC := src/main.cpp

# Files that need OGDF
OGDF_SRCS := src/baseline-ogdf/drawer.cpp

# Object files
COMMON_OBJS := $(COMMON_SRCS:%.cpp=$(OBJ_DIR)/%.o)
OGDF_OBJS := $(OGDF_SRCS:%.cpp=$(OBJ_DIR)/%.o)
STATS_OBJ := $(STATS_MAIN_SRC:%.cpp=$(OBJ_DIR)/%.o)
GEN_OBJ := $(GEN_MAIN_SRC:%.cpp=$(OBJ_DIR)/%.o)
MAIN_OBJ := $(MAIN_MAIN_SRC:%.cpp=$(OBJ_DIR)/%.o)

# Final output binaries
STATS_TARGET := stats
GEN_TARGET := gen
MAIN_TARGET := main

# Build all targets
all: $(STATS_TARGET) $(GEN_TARGET) $(MAIN_TARGET)

# Link stats executable (with OGDF)
$(STATS_TARGET): $(STATS_OBJ) $(COMMON_OBJS) $(OGDF_OBJS)
	$(COMPILER) $(CXXFLAGS) -o $@ $^ $(OGDF_LDFLAGS)

# Link gen executable (without OGDF)
$(GEN_TARGET): $(GEN_OBJ) $(COMMON_OBJS)
	$(COMPILER) $(CXXFLAGS) -o $@ $^

# Link gen executable (without OGDF)
$(MAIN_TARGET): $(MAIN_OBJ) $(COMMON_OBJS) $(OGDF_OBJS)
	$(COMPILER) $(CXXFLAGS) -o $@ $^ $(OGDF_LDFLAGS)

# Compile normal source files
$(COMMON_OBJS): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

# Compile main source files
$(STATS_OBJ) $(GEN_OBJ) $(MAIN_OBJ): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

# Compile OGDF-dependent source files
$(OGDF_OBJS): $(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) $(OGDF_INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(STATS_TARGET) $(GEN_TARGET) $(MAIN_TARGET)

.PHONY: all clean
