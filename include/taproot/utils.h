#pragma once
#include <iostream>
#include <sstream>
#include <vector>

bool isAllSpace(const std::string& input);
std::vector<std::string> tokenize(const std::string& input);
std::string toLower(std::string input);
std::string getTimestamp();

template <typename T>
std::string printVector(const std::vector<T>& vector) {
    /* Prints vectors in a neat format for display. */
    std::ostringstream buffer;
    for (const auto& elem : vector) {
        buffer << elem << "\n";
    }
    return buffer.str();
}

template <typename T>
std::string printVector(const std::vector<std::optional<T>>& vector) {
    /* Prints vectors in a neat format for display when working with optional types. */
    std::ostringstream buffer;
    for (const auto& elem : vector) {
        if (elem) {
            buffer << *elem << "\n";
        }
        else {
            buffer << "(null)\n";
        }
    }
    return buffer.str();
}

template <typename T>
bool contains(const std::vector<T>& vector, const T& value) {
    /* Checks if the vector contains a specific value as an element. */
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

template <typename T>
bool contains(const std::vector<std::optional<T>>& vector, const T& value) {
    /* Checks if the vector contains a specific value as an element when working with optional types. */
    return std::any_of(vector.begin(), vector.end(), [&value](const std::optional<T>& opt) {
        return opt && *opt == value;
    });
}