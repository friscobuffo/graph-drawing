#include "glucose.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdexcept>

std::string CONJUNCTIVE_NORMAL_FORM_FILE = "";
std::string OUTPUT_FILE = "";
std::string PROOF_FILE = "";

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
    remove(CONJUNCTIVE_NORMAL_FORM_FILE.c_str());
    remove(OUTPUT_FILE.c_str());
    remove(PROOF_FILE.c_str());
}

const GlucoseResult* launch_glucose(
    const std::string& conjunctive_normal_form_file,
    const std::string& output_file,
    const std::string& proof_file
) {
    CONJUNCTIVE_NORMAL_FORM_FILE = conjunctive_normal_form_file;
    OUTPUT_FILE = output_file;
    PROOF_FILE = proof_file;
    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("Failed to fork process");
    if (pid == 0) {
        // child process
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull == -1) {
            _exit(1);
        }
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);
        std::string proof_path("-certified-output=");
        proof_path += PROOF_FILE;
        execl("./glucose", "glucose", CONJUNCTIVE_NORMAL_FORM_FILE.c_str(), 
              OUTPUT_FILE.c_str(), "-certified", proof_path.c_str(), (char *)NULL);
        // if exec fails
        _exit(1);
    }
    // parent process
    int status;
    if (waitpid(pid, &status, 0) == -1)
        throw std::runtime_error("Failed to wait for process");
    const GlucoseResult* result = get_results();
    delete_glucose_temp_files();
    return result;
}

std::vector<std::string> get_proof() {
    std::ifstream file(PROOF_FILE.c_str());
    if (!file)
        throw std::runtime_error("Error: Could not open the file.");
    std::vector<std::string> proof_lines;
    std::string line;
    while (std::getline(file, line))
        proof_lines.push_back(line);
    return proof_lines;
}

const GlucoseResult* get_results() {
    std::ifstream file(OUTPUT_FILE.c_str());
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
    throw std::runtime_error("Error: the file is empty.");
}