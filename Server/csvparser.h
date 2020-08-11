#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include "Server.h"

struct Cell {
    const char * start;
    int size;
};

using fields_t = std::vector<Cell>;

const char* parseLine(const char* begin, const char* end, fields_t &fields) {

    Cell cell {nullptr, 0};

    while(begin != end) {
        if(*begin == '\n') {
            break;
        }
        if(*begin == ',') {
            cell.start = begin - cell.size;
            fields.push_back(cell);
            cell = {};
        } else {
            cell.size++;
        }

        begin++;
    }

    cell.start = begin - cell.size;
    fields.push_back(cell);

    return begin;
}

int csvParser(std::string tableName, const std::string &body) {
    auto pos = body.c_str();
    auto end = pos + body.size();

    fields_t fields;
    std::vector<std::string> columnsName;

    pos = parseLine(pos, end, fields);
    ++pos;

    std:: string tablePath = Server::dataPath + "/" + Server::defaultDB +"/" + tableName + "/";

    std::filesystem::create_directories(tablePath);

    std::vector< std::vector<char>> columnsData(fields.size());

    std::ofstream outfile (tablePath + "columns.txt",std::ofstream::binary);
    for(auto col : fields) {
        std::string colName(col.start, col.size);
        columnsName.push_back(colName);
        outfile << colName << '\n';
    }

    fields.clear();

    int count = 0;

    while(pos < end) {
        pos = parseLine(pos, end, fields);

        for(int i = 0; i < fields.size() && i < columnsData.size(); i++) {
            auto &field = fields[i];
            auto &column = columnsData[i];
            column.insert(column.end(), field.start, field.start + field.size);
            column.push_back('\n');
        }

        fields.clear();
        ++pos;
        ++count;
    }

    for(int i = 0; i < columnsName.size(); i++) {
        std::string fileName = tablePath + columnsName[i];
        std::ofstream colFile(fileName, std::ofstream::binary);
        colFile.write(&columnsData[i][0], columnsData[i].size());
    }

    std::ofstream countFile (tablePath + "count.txt", std::ofstream::binary);
    countFile << count;
    return count;
}
