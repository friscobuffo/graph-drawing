#include "sat/cnf_builder.hpp"

#include <fstream>
#include <iostream>


void CnfBuilder::add_clause(std::vector<int> clause) {
    for (int lit : clause)
        m_num_vars = std::max(m_num_vars, std::abs(lit));
    m_rows.push_back({
        CnfRowType::CLAUSE,
        std::move(clause),
        ""
    });
    m_num_clauses++;
}

void CnfBuilder::add_comment(const std::string& comment) {
    m_rows.push_back({
        CnfRowType::COMMENT,
        {},
        comment
    });
}

int CnfBuilder::get_number_of_variables() const {
    return m_num_vars;
}

int CnfBuilder::get_number_of_clauses() const {
    return m_num_clauses;
}

void CnfBuilder::convert_to_cnf(const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file) {
        std::cerr << "Error: Could not open file " << file_path << " for writing.\n";
        return;
    }
    file << "p cnf " << get_number_of_variables() << " " << get_number_of_clauses() << "\n";
    for (const auto& row : m_rows) {
        switch (row.m_type) {
        case CnfRowType::COMMENT:
            file << "c " << row.m_comment << "\n";
            break;
        case CnfRowType::CLAUSE:
            for (int lit : row.m_clause)
                file << lit << " ";
            file << "0\n";
            break;
        }
    }
    file.close();
}