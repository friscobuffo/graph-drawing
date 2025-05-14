#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include <iostream>
#include <vector>
#include <string>

struct CSVData {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

CSVData parse_csv(const std::string& filename);    

#endif