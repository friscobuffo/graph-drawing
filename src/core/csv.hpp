#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

struct CSVData {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

CSVData parse_csv(const std::string& filename);    

#endif