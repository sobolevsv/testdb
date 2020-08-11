#include "Server.h"
#include "csvparser.h"
#include "Interpreters/executeQuery.h"


void Server::processImportRequest(const httplib::Request& req, httplib::Response& res){
    auto numbers = req.matches[1];

    for(auto a : req.matches) {
        std::cout << a << std::endl;
    }
    int rowsCount = csvParser(req.body);
    res.set_content( std::to_string(rowsCount) + " rows imported", "text/plain");
}

void Server::processQueryRequest(const httplib::Request& req, httplib::Response& res) {
    BlockStreamPtr out;
    for(auto  &a : req.params) {
        if (a.first == "sql") {
            out = executeQuery(a.second);
            break;
        }
    }

    std::stringstream ss;

    try {
        while (*out) {
            auto block = out->pop();
            if (block->columns.empty())
                break;

            for (const auto & curCol: block->columns) {
                auto colName = curCol->alias.empty() ? curCol->columnName : curCol->alias;
                std::cout << colName << ',';
                ss << colName << ',';
            }
            std::cout << std::endl;
            ss << std::endl;
            for (int i = 0; i < block->columns[0]->data.size(); ++i) {
                for (const auto & col : block->columns) {
                    std::cout << col->data[i] << ',';
                    ss << col->data[i] << ',';
                }
                std::cout << std::endl;
                ss << std::endl;
            }
        }
    }
    catch (const char * err) {
        std::cerr << err << std::endl;
    }


    res.set_content( ss.str(), "text/plain");
}