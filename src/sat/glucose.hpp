#ifndef MY_GLUCOSE_H
#define MY_GLUCOSE_H

#include <string>
#include <vector>

enum class GlucoseResultType {
    SAT,
    UNSAT
};

struct GlucoseResult {
    GlucoseResultType result;
    std::vector<int> numbers;
    std::vector<std::string> proof_lines;
};

const GlucoseResult* launch_glucose();

#endif