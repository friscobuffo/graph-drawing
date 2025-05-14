#ifndef MY_GLUCOSE_H
#define MY_GLUCOSE_H

#include <string>
#include <vector>
#include "../core/utils.hpp"

enum GlucoseResultType {
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

const GlucoseResult* launch_glucose(
    const std::string& conjunctive_normal_form_file,
    const std::string& output_file,
    const std::string& proof_file
);

#endif