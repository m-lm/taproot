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