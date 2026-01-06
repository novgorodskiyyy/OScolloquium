#include "TaskManager.hpp"
#include <sstream>

std::string TaskManager::getAllTasksJson() {
    std::lock_guard<std::mutex> lock(mtx);
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < tasks.size(); ++i) {
        oss << tasks[i].toJson();
        if (i != tasks.size() - 1) oss << ",";
    }
    oss << "]";
    return oss.str();
}

std::string TaskManager::getTaskJson(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& task : tasks) {
        if (task.id == id) {
            return task.toJson();
        }
    }
    return "{}";
}

int TaskManager::addTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mtx);
    Task newTask = task;
    newTask.id = nextId++;
    tasks.push_back(newTask);
    return newTask.id;
}

bool TaskManager::updateTask(int id, const Task& updatedTask) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& task : tasks) {
        if (task.id == id) {
            task.title = updatedTask.title;
            task.description = updatedTask.description;
            task.status = updatedTask.status;
            return true;
        }
    }
    return false;
}


bool TaskManager::partialUpdate(int id, const Task& partialTask) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& task : tasks) {
        if (task.id == id) {
          

            if (!partialTask.title.empty()) {
                task.title = partialTask.title;
            }
            if (!partialTask.description.empty()) {
                task.description = partialTask.description;
            }
            if (!partialTask.status.empty()) {
                task.status = partialTask.status;
            }
            return true;
        }
    }
    return false;
}

bool TaskManager::deleteTask(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        if (it->id == id) {
            tasks.erase(it);
            return true;
        }
    }
    return false;
}