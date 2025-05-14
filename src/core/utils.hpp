#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <vector>
#include <iostream>
#include <tuple>
#include <string>
#include <cassert>
#include <fstream>
#include <sstream>
#include <memory>

template <typename Iterable>
void print_iterable(const Iterable& container, const std::string& end = "\n") {
    std::cout << "[ ";
    for (const auto& elem : container)
        std::cout << elem << " ";
    std::cout << "]" << end;
}

template <typename T>
void print_array(T array[], int size, const std::string& end = "\n") {
    std::cout << "[ ";
    for (int i = 0; i < size; ++i)
        std::cout << array[i] << " ";
    std::cout << "]" << end;
}

inline void save_string_to_file(std::string filename, std::string& content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
    } else
        std::cerr << "Unable to open file" << std::endl;
}

enum Color {
    RED,
    BLUE,
    BLACK,
    RED_AND_BLUE,
    NONE,
    ANY,
};

inline const std::string color_to_string(const Color color) {
    switch (color) {
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        case Color::BLACK: return "black";
        default: assert(false);
    }
}

#endif