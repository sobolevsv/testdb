#include "Server.h"
#include "csvparser.h"
#include "../Interpreters/executeQuery.h"


void Server::processImportRequest(const httplib::Request& req, httplib::Response& res){
    auto numbers = req.matches[1];

    for(auto a : req.matches) {
        std::cout << a << std::endl;
    }
    int rowsCount = csvParser(req.body);
    res.set_content( std::to_string(rowsCount) + " rows imported", "text/plain");
}

void Server::processQueryRequest(const httplib::Request& req, httplib::Response& res) {
    for(auto  &a : req.params) {
        std::cout << a.first << ", 2:" << a.second << std::endl;
        if (a.first == "sql") {
            executeQuery(a.second);
        }
    }

    std::cout << req.body << std::endl;
}