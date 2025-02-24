#include "glucose.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>

std::string GlucoseResult::to_string() const {
    std::string r = result == GlucoseResultType::SAT ? "SAT" : "UNSAT";
    std::string numbers_str = "Numbers: ";
    for (int num : numbers)
        numbers_str += std::to_string(num) + " ";
    std::string proof_str = "Proof:\n";
    for (const std::string& line : proof_lines)
        proof_str += line + "\n";
    return r + "\n" + numbers_str + "\n" + proof_str;
}

void GlucoseResult::print() const {
    std::cout << to_string() << std::endl;
}

const GlucoseResult* get_results();

void delete_glucose_temp_files() {
    remove(".conjunctive_normal_form.cnf");
    remove(".output.txt");
    remove(".proof.txt");
}

const GlucoseResult* launch_glucose() {
    FILE* pipe = popen("./glucose .conjunctive_normal_form.cnf .output.txt -certified -certified-output=.proof.txt", "r");
    if (!pipe) {
        std::cerr << "Failed to run executable" << std::endl;
        return nullptr;
    }
    pclose(pipe);
    std::cout << "glucose finished\n";
    std::cout << "getting results\n";
    const GlucoseResult* result = get_results();
    std::cout << "deleting temp files\n";
    delete_glucose_temp_files();
    std::cout << "deleted temp files\n";
    return result;
}

std::vector<std::string> get_proof() {
    std::ifstream file(".proof.txt");
    if (!file)
        throw std::runtime_error("Error: Could not open the file.");
    std::vector<std::string> proof_lines;
    std::string line;
    while (std::getline(file, line))
        proof_lines.push_back(line);
    return proof_lines;
}

const GlucoseResult* get_results() {
    std::ifstream file(".output.txt");
    if (!file)
        throw std::runtime_error("Error: Could not open the file.");
    std::string line;
    if (std::getline(file, line)) {
        if (line == "UNSAT")
            return new GlucoseResult{GlucoseResultType::UNSAT, {}, get_proof()};
        std::istringstream iss(line);
        std::vector<int> numbers;
        int num;
        while (iss >> num)
            numbers.push_back(num);
        return new GlucoseResult{GlucoseResultType::SAT, numbers, get_proof()};
    }
    return nullptr;
}