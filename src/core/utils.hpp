#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <vector>
#include <iostream>
#include <string>
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

void save_string_to_file(const std::string& filename, const std::string& content);

enum class Color {
    RED,
    BLUE,
    BLACK,
    RED_AND_BLUE,
    NONE,
    ANY,
};

const std::string color_to_string(const Color color);

std::string get_unique_filename(const std::string& base_filename);

#endif