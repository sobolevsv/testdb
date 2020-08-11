#include "Server.h"
#include "csvparser.h"
#include "Interpreters/executeQuery.h"
#include "Common/Exception.h"

std::string Server::dataPath = "data/";
std::string Server::defaultDB = "test";

void Server::processImportRequest(const httplib::Request& req, httplib::Response& res) {
    std::string tableName;
    if (req.matches.size() > 1)
        tableName = req.matches[1];

    int rowsCount = csvParser(tableName, req.body);
    res.set_content( std::to_string(rowsCount) + " rows imported", "text/plain");
}

void Server::processQueryRequest(const httplib::Request& req, httplib::Response& res) {
    BlockStreamPtr out;

    try {
        for(auto  &a : req.params) {
            if (a.first == "sql") {
                out = executeQuery(a.second);
                break;
            }
        }
    }
    catch (const Exception & err) {
        std::cerr << err.what() << std::endl;
        res.set_content( err.what(), "text/plain");
        res.status = 400;
        return;
    }

    std::stringstream ss;

    try {
        // pull blocks from stream until it closed and empty
        while (*out) {
            auto block = out->pop();
            if (block->columns.empty())
                break;

            bool first = true;
            // print columns header
            for (const auto & curCol: block->columns) {
                auto colName = curCol->alias.empty() ? curCol->columnName : curCol->alias;
                if (!first) {
                    ss <<  ',';
                    std::cout <<  ',';
                }
                std::cout << colName;
                ss << colName;
                first = false;
            }
            std::cout << std::endl;
            ss << std::endl;

            // print columns data
            for (int i = 0; i < block->columns[0]->data.size(); ++i) {
                first = true;
                for (const auto & col : block->columns) {
                    if (!first) {
                        std::cout << ',';
                        ss << ',';
                    }
                    std::cout << col->data[i];
                    ss << col->data[i];
                    first = false;
                }
                std::cout << std::endl;
                ss << std::endl;
            }
        }
    }
    catch (const Exception & err) {
        std::cerr << err.what() << std::endl;
        res.set_content( err.what(), "text/plain");
        res.status = 400;
    }

    res.set_content( ss.str(), "text/plain");
}