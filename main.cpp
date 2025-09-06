//include httplib for protocol http request
#include <httplib.h>
#include <json.hpp>

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <iostream>

using json = nlohmann::json;

struct Todo{
    int id;
    std::string title;
    bool completed;
};

inline void to_json(json& j, const Todo& t){
    j = json{{ "id", t.id }, {"title", t.title}, {"completed", t.completed}};
}

inline void from_json(const json& j, Todo& t){
    if(j.contains("id")) t.id = j.at("id").get<int>();
    if(j.contains("title")) t.title = j.at("title").get<std::string>();
    if(j.contains("completed")) t.completed = j.at("completed").get<bool>();
}

std::unordered_map<int, Todo> todos;
std::mutex todos_mutes;
std::atomic<int> nextId(1);

int main() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Todo API (C++ / cpp-httplib)\n", "text/plain");
    });

    svr.Get("/todos", [](const httplib::Request&, httplib::Response& res){
       json arr = json::array();
       {
            std::lock_guard<std::mutex> lock(todos_mutes);
            for(auto &p : todos){
                arr.push_back(p.second);
            }
       }
       res.set_content(arr.dump(2), "application/json");
    });

    svr.Get(R"(/todos/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        std::lock_guard<std::mutex> lock(todos_mutes);
        auto it = todos.find(id);

        if(it == todos.end()) {
            res.status = 404;
            res.set_content(R"({"error" : "Not Found})", "application/json");
            return;
        }
        json j = it->second;
        res.set_content(j.dump(2), "application/json");
    });

    svr.Post("/todos", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            if (!body.contains("title")) {
                res.status = 400;
                res.set_content(R"({"error":"Missing field 'title'"})", "application/json");
                return;
            }
            Todo t;
            t.id = nextId++;
            t.title = body.at("title").get<std::string>();
            t.completed = body.value("completed", false);

            {
                std::lock_guard<std::mutex> lock(todos_mutes);
                todos[t.id] = t;
            }

            json j = t;
            res.status = 201;
            res.set_content(j.dump(2), "application/json");
            // optionally set Location header
            res.set_header("Location", "/todos/" + std::to_string(t.id));
        } catch (const std::exception& ex) {
            res.status = 400;
            json err = { {"error", "Invalid JSON"}, {"details", ex.what()} };
            res.set_content(err.dump(2), "application/json");
        }
    });

    svr.Put(R"(/todos/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        try {
            json body = json::parse(req.body);
            std::lock_guard<std::mutex> lock(todos_mutes);
            auto it = todos.find(id);
            if (it == todos.end()) {
                res.status = 404;
                res.set_content(R"({"error":"Not found"})", "application/json");
                return;
            }
            if (body.contains("title")) it->second.title = body.at("title").get<std::string>();
            if (body.contains("completed")) it->second.completed = body.at("completed").get<bool>();

            json j = it->second;
            res.set_content(j.dump(2), "application/json");
        } catch (const std::exception& ex) {
            res.status = 400;
            json err = { {"error", "Invalid JSON"}, {"details", ex.what()} };
            res.set_content(err.dump(2), "application/json");
        }
    });

    svr.Delete(R"(/todos/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        std::lock_guard<std::mutex> lock(todos_mutes);
        auto it = todos.find(id);
        if (it == todos.end()) {
            res.status = 404;
            res.set_content(R"({"error":"Not found"})", "application/json");
            return;
        }
        todos.erase(it);
        res.status = 204;
        res.set_content("", "text/plain");
    });


    int port = 8888;
    std::cout << "Starting server on http://0.0.0.0:" << port << "\n";
    svr.listen("0.0.0.0", port);

    return 0;
}
