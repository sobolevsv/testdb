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
        bool queryFound = false;
        for(auto  &a : req.params) {
            if (a.first == "sql") {
                queryFound = true;
                out = executeQuery(a.second);
                break;
            }
        }
        if(!queryFound) {
            const auto errMsg = "wrong request; expected /query?sql= SELECT ...";
            std::cerr  << std::endl;
            res.set_content( errMsg, "text/plain");
            res.status = 400;
            return;
        }
    }
    catch (const Exception & err) {
        std::cerr << err.what() << std::endl;
        res.set_content( err.what(), "text/plain");
        res.status = 400;
        return;
    }

    if(!out) {
        std::cerr << "internal error: output stream is nil"  << std::endl;
        res.set_content( "internal error", "text/plain");
        res.status = 500;
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
                }
                ss << colName;
                first = false;
            }
            ss << std::endl;

            // print columns data
            for (int i = 0; i < block->columns[0]->data.size(); ++i) {
                first = true;
                for (const auto & col : block->columns) {
                    if (!first) {
                        ss << ',';
                    }
                    ss << col->data[i];
                    first = false;
                }
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