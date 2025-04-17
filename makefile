COMPILER = g++
CXXFLAGS = -std=c++20 -O3 -march=native

# OGDF flags (only for stats)
OGDF_INCLUDES = -IOGDF/include
OGDF_LIBS = -LOGDF -lOGDF -lCOIN

# Common sources (shared between both binaries)
COMMON_SRCS = src/sat/glucose.cpp \
              src/sat/cnf_builder.cpp \
              src/orthogonal/shape.cpp \
              src/orthogonal/shape_builder.cpp \
              src/drawing/polygon.cpp \
              src/drawing/svg_drawer.cpp \
              src/orthogonal/drawing_builder.cpp \
              src/config/config.cpp \
			  src/core/graph/generators.cpp

# Program-specific sources
MAIN_SRCS = src/main.cpp
STATS_SRCS = src/stats.cpp src/baseline-ogdf/drawer.cpp
GEN_SRCS = src/gen.cpp

# Object files (all in obj/)
MAIN_OBJS = $(MAIN_SRCS:%.cpp=obj/%.o) $(COMMON_SRCS:%.cpp=obj/%.o)
STATS_OBJS = $(STATS_SRCS:%.cpp=obj/%.o) $(COMMON_SRCS:%.cpp=obj/%.o)
GEN_OBJS = $(GEN_SRCS:%.cpp=obj/%.o) $(COMMON_SRCS:%.cpp=obj/%.o)

# Default: build both
all: main stats gen

# Main program (no OGDF)
main: $(MAIN_OBJS)
	$(COMPILER) $(CXXFLAGS) $^ -o $@

# Gen program (no OGDF)
gen: $(GEN_OBJS)
	$(COMPILER) $(CXXFLAGS) $^ -o $@

# Stats program (with OGDF)
stats: $(STATS_OBJS)
	$(COMPILER) $(CXXFLAGS) $(OGDF_INCLUDES) $^ -o $@ $(OGDF_LIBS)

# Rule: compile .cpp → .o (shared objects)
obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

# Rule: compile stats-specific objects with OGDF flags
obj/src/baseline-ogdf/%.o: src/baseline-ogdf/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(CXXFLAGS) $(OGDF_INCLUDES) -c $< -o $@

clean:
	rm -rf obj main stats