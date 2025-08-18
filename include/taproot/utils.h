#pragma once
#include <iostream>
#include <vector>

bool isAllSpace(const std::string& input);
std::vector<std::string> tokenize(const std::string& input);
std::string getTimestamp();

template <typename T>
void printVector(const std::vector<T>& vector) {
    // Prints vectors in a neat format for display
    for (const auto& elem : vector) {
        std::cout << elem << std::endl;
    }
}

template <typename T>
void printVector(const std::vector<std::optional<T>>& vector) {
    // Prints vectors in a neat format for display when working with optional types
    for (const auto& elem : vector) {
        if (elem) {
            std::cout << *elem << std::endl;
        }
        else {
            std::cout << "(null)" << std::endl;
        }
    }
}

template <typename T>
bool contains(const std::vector<T>& vector, const T& value) {
    // Checks if the vector contains value as an element
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

template <typename T>
bool contains(const std::vector<std::optional<T>>& vector, const T& value) {
    // Checks if the vector contains value as an element when working with optional types
    return std::any_of(vector.begin(), vector.end(), [&value](const std::optional<T>& opt) {
        return opt && *opt == value;
    });
}