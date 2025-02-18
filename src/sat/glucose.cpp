#include "glucose.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>

const Result* get_results();

const Result* launch_glucose() {
    FILE* pipe = popen("./glucose .conjunctive_normal_form.cnf .output.txt -certified -certified-output=.proof.txt", "r");
    if (!pipe) {
        std::cerr << "Failed to run executable" << std::endl;
        return nullptr;
    }
    fclose(pipe);
    return get_results();
}

std::string get_proof() {
    std::ifstream file(".proof.txt");
    if (!file) {
        std::cerr << "Error: Could not open the file.\n";
        return "";
    }
    std::string line;
    std::string proof;
    while (std::getline(file, line))
        proof += line + "\n";
    return proof;
}

const Result* get_results() {
    std::ifstream file(".output.txt"); // Open the file
    if (!file) {
        std::cerr << "Error: Could not open the file.\n";
        return nullptr;
    }
    std::string line;
    if (std::getline(file, line)) {
        if (line == "UNSAT")
            return new Result{ResultType::UNSAT, {}, get_proof()};
        std::istringstream iss(line);
        std::vector<int> numbers;
        int num;
        while (iss >> num)
            numbers.push_back(num);
        return new Result{ResultType::SAT, numbers, get_proof()};
    }
    return nullptr;
}