#pragma once
#include "HTTPServer.hpp"
#include "TaskManager.hpp"
#include "Task.hpp"
#include <sstream>
#include <regex>
#include <algorithm>

class Router {
private:
    TaskManager taskManager;

    int extractId(const std::string& path) {
        try {
            size_t last_slash = path.find_last_of('/');
            if (last_slash != std::string::npos) {
                std::string id_str = path.substr(last_slash + 1);
                return std::stoi(id_str);
            }
        }
        catch (...) {
            return -1;
        }
        return -1;
    }

public:
    void setup_routes(HTTPServer& server) {
        server.add_route("GET /tasks", [this](const Request& req) {
            Response res;
            res.body = taskManager.getAllTasksJson();
            return res;
            });

        server.add_route("POST /tasks", [this](const Request& req) {
            Response res;
            Task task = Task::fromJson(req.body);
            int id = taskManager.addTask(task);
            task.id = id;
            res.status = 201;
            res.body = task.toJson();
            return res;
            });

        server.add_route("GET /tasks/", [this](const Request& req) {
            Response res;
            int id = extractId(req.path);
            if (id <= 0) {
                res.status = 400;
                res.body = "{\"error\":\"Invalid ID\"}";
                return res;
            }
            std::string taskJson = taskManager.getTaskJson(id);
            if (taskJson == "{}") {
                res.status = 404;
                res.body = "{\"error\":\"Task not found\"}";
            }
            else {
                res.body = taskJson;
            }
            return res;
            });

        server.add_route("PUT /tasks/", [this](const Request& req) {
            Response res;
            int id = extractId(req.path);
            if (id <= 0) {
                res.status = 400;
                res.body = "{\"error\":\"Invalid ID\"}";
                return res;
            }

           
            std::string currentJson = taskManager.getTaskJson(id);
            if (currentJson == "{}") {
                res.status = 404;
                res.body = "{\"error\":\"Task not found\"}";
                return res;
            }

            Task currentTask = Task::fromJson(currentJson);
            Task updatedFields = Task::fromJson(req.body);


            if (!updatedFields.title.empty()) {
                currentTask.title = updatedFields.title;
            }
            if (!updatedFields.description.empty()) {
                currentTask.description = updatedFields.description;
            }
            if (!updatedFields.status.empty()) {
                currentTask.status = updatedFields.status;
            }

            currentTask.id = id;
            if (taskManager.updateTask(id, currentTask)) {
                res.body = currentTask.toJson();
            }
            else {
                res.status = 404;
                res.body = "{\"error\":\"Task not found\"}";
            }
            return res;
            });


        server.add_route("PATCH /tasks/", [this](const Request& req) {
            Response res;
            int id = extractId(req.path);
            if (id <= 0) {
                res.status = 400;
                res.body = "{\"error\":\"Invalid ID\"}";
                return res;
            }

            Task partialTask = Task::fromJson(req.body);
            if (taskManager.partialUpdate(id, partialTask)) {
                std::string taskJson = taskManager.getTaskJson(id);
                res.body = taskJson;
            }
            else {
                res.status = 404;
                res.body = "{\"error\":\"Task not found\"}";
            }
            return res;
            });

        server.add_route("DELETE /tasks/", [this](const Request& req) {
            Response res;
            int id = extractId(req.path);
            if (id <= 0) {
                res.status = 400;
                res.body = "{\"error\":\"Invalid ID\"}";
                return res;
            }
            if (taskManager.deleteTask(id)) {
                res.body = "{\"message\":\"Task deleted\"}";
            }
            else {
                res.status = 404;
                res.body = "{\"error\":\"Task not found\"}";
            }
            return res;
            });
    }
};