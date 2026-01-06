#pragma once
#include "Task.hpp"
#include <vector>
#include <mutex>

class TaskManager {
private:
    std::vector<Task> tasks;
    std::mutex mtx;
    int nextId = 1;

public:
    std::string getAllTasksJson();
    std::string getTaskJson(int id);
    int addTask(const Task& task);
    bool updateTask(int id, const Task& updatedTask);
    bool partialUpdate(int id, const Task& partialTask);  
    bool deleteTask(int id);
};