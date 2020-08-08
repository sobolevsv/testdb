#pragma once

#include <string>
#include <vector>
#include <sstream>

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

int csvParser(const std::string &body) {
    auto pos = body.c_str();
    auto end = pos + body.size();

    fields_t columnsName;

    pos = parseLine(pos, end, columnsName);
    ++pos;

    std::vector< std::shared_ptr<std::ofstream>> columnFiles;
    std::vector< std::vector<char>> columnStream;

    std::ofstream outfile ("data/column.txt",std::ofstream::binary);
    for(auto col : columnsName) {
        outfile.write(col.start, col.size);
        outfile.put('\n');
        std::string fileName(col.start, col.size);
        fileName = "data/" + fileName;
        columnFiles.push_back( std::make_shared<std::ofstream>(fileName, std::ofstream::binary));
        columnStream.push_back( std::vector<char>());
    }
    outfile.close();

    fields_t fields;

    int count = 0;

    while(pos < end) {
        pos = parseLine(pos, end, fields);

        for(int i = 0; i < fields.size(); i++) {
//            columnFiles[i]->write(fields[i].start, fields[i].size);
            //columnFiles[i]->put('\n');

            columnStream[i].insert(columnStream[i].end(), fields[i].start, fields[i].start +  fields[i].size);
        }

        fields.clear();
        ++pos;
        ++count;
    }

    for(int i = 0; i < columnsName.size(); i++) {
        columnFiles[i]->write(&columnStream[i][0], columnStream[i].size());

        //columnStream[i]->insert(columnStream[i]->end(), fields[i].start, fields[i].start +  fields[i].size);
    }

    std::ofstream countfile ("data/count.txt",std::ofstream::binary);
    countfile << count;
    return count;
}
