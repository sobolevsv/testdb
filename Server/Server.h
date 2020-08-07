#pragma once

#include "httplib.h"

class Server : public httplib::Server {
public:
    Server() {

    }

    void init() {
        Post(R"(/post/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
            processImportRequest(req, res);
        });

        Get("/query", [&](const httplib::Request& req, httplib::Response& res) {
            processQueryRequest(req, res);
        });
    }

private:

    void processImportRequest(const httplib::Request& req, httplib::Response& res){
        auto numbers = req.matches[1];
        res.set_content(numbers, "text/plain");

        for(auto a : req.matches) {
            std::cout << a << std::endl;
        }
        std::cout << req.body << std::endl;
    }

    void processQueryRequest(const httplib::Request& req, httplib::Response& res) {
        for(auto  &a : req.params) {
            std::cout << a.first << ", 2:" << a.second << std::endl;
        }

        std::cout << req.body << std::endl;
    }

};

