#ifndef MY_CNF_BUILDER_H
#define MY_CNF_BUILDER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

class CNFBuilder {
private:
    int m_num_vars = 0;
    std::vector<std::vector<int>> m_clauses;
public:
    void add_clause(std::vector<int> clause) {
        for (int lit : clause)
            m_num_vars = std::max(m_num_vars, std::abs(lit));
        m_clauses.push_back(std::move(clause));
    }
    int get_number_of_variables() const {
        return m_num_vars;
    }
    int get_number_of_clauses() const {
        return m_clauses.size();
    }
    void convert_to_cnf(const std::string& file_path) const {
        std::ofstream file(file_path);
        if (!file) {
            std::cerr << "Error: Could not open file " << file_path << " for writing.\n";
            return;
        }
        file << "p cnf " << m_num_vars << " " << m_clauses.size() << "\n";
        for (const auto& clause : m_clauses) {
            for (int lit : clause)
                file << lit << " ";
            file << "0\n";
        }
        file.close();
    }
};

#endif