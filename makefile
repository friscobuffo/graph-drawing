COMPILER := g++
OBJ_DIR := obj

# Directory flags
OGDF_INCLUDES := -IOGDF/include
CXXFLAGS := -std=c++20 -O3 -march=native

# OGDF flags
OGDF_LDFLAGS := -LOGDF -lOGDF -lCOIN

# Source files
COMMON_SRCS := \
	src/sat/glucose.cpp \
	src/sat/cnf_builder.cpp \
	src/orthogonal/shape.cpp \
	src/orthogonal/shape_builder.cpp \
	src/drawing/polygon.cpp \
	src/drawing/svg_drawer.cpp \
	src/orthogonal/drawing_builder.cpp \
    src/config/config.cpp \
	src/core/graph/generators.cpp

# Files that need OGDF
OGDF_SRCS := src/baseline-ogdf/drawer.cpp

STATS_SRC := src/stats.cpp
MAIN_SRC := src/main.cpp
GEN_SRC := src/gen.cpp

# Object files
COMMON_OBJS := $(COMMON_SRCS:%.cpp=$(OBJ_DIR)/%.o)
OGDF_OBJS := $(OGDF_SRCS:%.cpp=$(OBJ_DIR)/%.o)

MAIN_OBJ := $(MAIN_SRC:%.cpp=$(OBJ_DIR)/%.o)
STATS_OBJ := $(STATS_SRC:%.cpp=$(OBJ_DIR)/%.o)
GEN_OBJ := $(GEN_SRC:%.cpp=$(OBJ_DIR)/%.o)

# Final targets
TARGETS := main stats gen

# Default: build all
all: $(TARGETS)

# Main program (no OGDF)
main: $(MAIN_OBJ) $(COMMON_OBJS)
	$(COMPILER) $(CXXFLAGS) $^ -o $@

# Gen program (no OGDF)
gen: $(GEN_OBJ) $(COMMON_OBJS)
	$(COMPILER) $(CXXFLAGS) $^ -o $@

# Stats program (with OGDF)
stats: $(STATS_OBJ) $(COMMON_OBJS) $(OGDF_OBJS)
	$(COMPILER) $(CXXFLAGS) $^ -o $@ $(OGDF_LDFLAGS)

# Compile normal source files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

# Compile OGDF-dependent source files
$(OBJ_DIR)/src/baseline-ogdf/%.o: src/baseline-ogdf/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) $(OGDF_INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGETS)

.PHONY: all clean $(TARGETS)