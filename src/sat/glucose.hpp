#ifndef MY_GLUCOSE_H
#define MY_GLUCOSE_H

#include <string>
#include <vector>

enum class ResultType {
    SAT,
    UNSAT
};

struct Result {
    ResultType result;
    std::vector<int> numbers;
    std::string proof;
};

const Result* launch_glucose();

#endif