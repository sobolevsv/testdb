#pragma once

#include <string>
#include <vector>
#include <sstream>

struct Field {
    const char * start;
    int size;
};

using fields_t = std::vector<Field>;

const char* parseLine(const char* begin, const char* end, fields_t &fields) {

    Field cell {nullptr, 0};

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

int csvParser(const std::string &body) {
    auto pos = body.c_str();
    auto end = pos + body.size();

    fields_t fields;
    std::vector<std::string> columnsName;

    pos = parseLine(pos, end, fields);
    ++pos;

    std::vector< std::vector<char>> columnsData(fields.size());

    std::ofstream outfile ("data/column.txt",std::ofstream::binary);
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
        }

        fields.clear();
        ++pos;
        ++count;
    }

    for(int i = 0; i < columnsName.size(); i++) {
        std::string fileName = "data/" + columnsName[i];
        std::ofstream colFile(fileName, std::ofstream::binary);
        colFile.write(&columnsData[i][0], columnsData[i].size());
    }

    std::ofstream countFile ("data/count.txt", std::ofstream::binary);
    countFile << count;
    return count;
}