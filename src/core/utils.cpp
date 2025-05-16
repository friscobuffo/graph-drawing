#include "utils.hpp"

#include <cstdlib> // For mkstemp, system, remove
#include <unistd.h> // For close
#include <fstream>  // To read the file later
#include <sstream>  // For std::stringstream
#include <iostream> // For std::cerr

#ifdef __linux__
const std::string TEMPORARY_FOLDER = "/dev/shm/";
#elif __APPLE__
const std::string TEMPORARY_FOLDER = "/tmp/";
#endif

std::string get_unique_filename(const std::string& base_filename) {
    // Create a unique temporary file
    std::string filename_template = TEMPORARY_FOLDER + base_filename + "_XXXXXX";
    int fd = mkstemp(filename_template.data());
    if (fd == -1) {
        perror("Failed to create unique temporary file");
        throw std::runtime_error("Failed to create unique temporary file");
    }
    close(fd); // Close the file descriptor
    return filename_template;
}

void save_string_to_file(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
    } else
        std::cerr << "Unable to open file" << std::endl;
}
