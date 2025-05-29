cmake_minimum_required(VERSION 3.16)
project(YourProject LANGUAGES CXX)

# Set C++ standard and optimization flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/OGDF/include)
link_directories(${CMAKE_SOURCE_DIR}/OGDF)

# Common source files (used in all executables)
set(COMMON_SRCS
    src/sat/glucose.cpp
    src/sat/cnf_builder.cpp
    src/orthogonal/shape/shape.cpp
    src/orthogonal/shape/shape_builder.cpp
    src/orthogonal/shape/variables_handler.cpp
    src/orthogonal/shape/clauses_functions.cpp
    src/drawing/polygon.cpp
    src/core/graph/graphs_algorithms.cpp
    src/core/graph/graph.cpp
    src/core/graph/file_loader.cpp
    src/core/tree/tree.cpp
    src/core/tree/tree_algorithms.cpp
    src/drawing/svg_drawer.cpp
    src/orthogonal/drawing_builder.cpp
    src/orthogonal/drawing_stats.cpp
    src/config/config.cpp
    src/core/graph/generators.cpp
    src/orthogonal/file_loader.cpp
    src/core/utils.cpp
    src/core/csv.cpp
    src/baseline-ogdf/drawer.cpp
)

# Create static library with common code
add_library(core STATIC ${COMMON_SRCS})

# Ensure core links against external dependencies
target_link_libraries(core PUBLIC OGDF COIN)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Create executables with only their own entry point
add_executable(main src/main.cpp)
add_executable(stats src/stats.cpp)
add_executable(gen src/gen.cpp)

# Link executables to shared core library
target_link_libraries(main PRIVATE core)
target_link_libraries(stats PRIVATE core)
target_link_libraries(gen PRIVATE core)
