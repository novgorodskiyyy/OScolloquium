#pragma once
#include <string>

struct Task {
    int id;
    std::string title;
    std::string description;
    std::string status;

    Task();
    Task(int id, const std::string& title, const std::string& description, const std::string& status);
    std::string toJson() const;
    static Task fromJson(const std::string& json);
};