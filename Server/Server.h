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

    void processImportRequest(const httplib::Request& req, httplib::Response& res);
    void processQueryRequest(const httplib::Request& req, httplib::Response& res) ;

};

