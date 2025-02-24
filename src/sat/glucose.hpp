#ifndef MY_GLUCOSE_H
#define MY_GLUCOSE_H

#include <string>
#include <vector>
#include "../core/utils.hpp"

enum class GlucoseResultType {
    SAT,
    UNSAT
};

struct GlucoseResult {
    GlucoseResultType result;
    std::vector<int> numbers;
    std::vector<std::string> proof_lines;
    std::string to_string() const;
    void print() const;
};

static_assert(PrintTrait<GlucoseResult>);

const GlucoseResult* launch_glucose();

#endif