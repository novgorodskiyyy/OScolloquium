#include "Task.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

Task::Task() : id(0), status("todo") {}
Task::Task(int id, const std::string& title, const std::string& description, const std::string& status)
    : id(id), title(title), description(description), status(status) {
}

std::string Task::toJson() const {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << id << ","
        << "\"title\":\"" << title << "\","
        << "\"description\":\"" << description << "\","
        << "\"status\":\"" << status << "\""
        << "}";
    return oss.str();
}

Task Task::fromJson(const std::string& json) {
    Task task;
   
    std::string fields[] = { "\"id\"", "\"title\"", "\"description\"", "\"status\"" };
    for (int i = 0; i < 4; i++) {
        size_t pos = json.find(fields[i]);
        if (pos == std::string::npos) continue;

        size_t colon_pos = json.find(':', pos);
        if (colon_pos == std::string::npos) continue;

        size_t value_start = colon_pos + 1;
        while (value_start < json.length() &&
            (json[value_start] == ' ' || json[value_start] == '\t' ||
                json[value_start] == '\n' || json[value_start] == '\r')) {
            value_start++;
        }

        if (value_start >= json.length()) continue;

        if (fields[i] == "\"id\"") {
            size_t value_end = json.find_first_of(",}", value_start);
            if (value_end == std::string::npos) continue;

            std::string value = json.substr(value_start, value_end - value_start);
            value.erase(0, value.find_first_not_of(" \t\n\r"));
            value.erase(value.find_last_not_of(" \t\n\r") + 1);

            try {
                task.id = std::stoi(value);
            }
            catch (...) {
                task.id = 0;
            }
        }
        else {
            if (json[value_start] != '"') continue;

            size_t quote_start = value_start + 1;
            size_t quote_end = quote_start;
            bool escaped = false;

            while (quote_end < json.length()) {
                if (json[quote_end] == '\\' && !escaped) {
                    escaped = true;
                }
                else if (json[quote_end] == '"' && !escaped) {
                    break;
                }
                else {
                    escaped = false;
                }
                quote_end++;
            }

            if (quote_end >= json.length()) continue;

            std::string value = json.substr(quote_start, quote_end - quote_start);

            if (fields[i] == "\"title\"") {
                task.title = value;
            }
            else if (fields[i] == "\"description\"") {
                task.description = value;
            }
            else if (fields[i] == "\"status\"") {
                task.status = value;
            }
        }
    }

    return task; 
}