#ifndef MY_CNF_BUILDER_H
#define MY_CNF_BUILDER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

class CNFBuilder {
private:
    std::vector<std::vector<int>> clauses;

public:
    void add_clause(std::vector<int> clause) {
        clauses.push_back(std::move(clause));
    }
    void convert_to_cnf(const std::string& file_path) const {
        std::ofstream file(file_path);
        if (!file) {
            std::cerr << "Error: Could not open file " << file_path << " for writing.\n";
            return;
        }
        int num_vars = 0;
        for (const auto& clause : clauses)
            for (int lit : clause)
                num_vars = std::max(num_vars, std::abs(lit));
        file << "p cnf " << num_vars << " " << clauses.size() << "\n";
        for (const auto& clause : clauses) {
            for (int lit : clause)
                file << lit << " ";
            file << "0\n";
        }
        file.close();
    }
};

#endif