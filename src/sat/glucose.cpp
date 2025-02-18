#include "glucose.hpp"

#include <cstdio>
#include <iostream>
#include <cstring>

int launch_glucose() {
    FILE* pipe = popen("./glucose .conjunctive_normal_form.cnf .output.txt -certified -certified-output=daje", "r");
    if (!pipe) {
        std::cerr << "Failed to run executable" << std::endl;
        return 1;
    }

    // char buffer[128];
    // while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    //     std::cout << buffer;  // Print output
    // }

    fclose(pipe);
    return 0;
}