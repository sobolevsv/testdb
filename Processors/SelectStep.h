#pragma once

#include <string>
#include <fstream>
#include <Server/Server.h>
#include <filesystem>
#include "DataStreams/Block.h"
#include "Parser/ASTIdentifier.h"
#include "Parser/ASTFunction.h"
#include "DataStreams/BlockStream.h"

std::vector<std::string> data;
std::string columnName;
std::string alias;

ColumnList makeColumnList(ASTIdentifierList columnsName) {
    ColumnList res;
    for (auto a: columnsName) {
        auto col = std::make_shared<Column>();
        col->columnName = a->shortName();
        col->alias = a->alias;
        res.push_back(col);
    }
    return res;
}

BlockStreamPtr SelectStep(ColumnList& columnsList, ASTIdentifierPtr table) {

    BlockStreamPtr out = std::make_shared<BlockStream>();

    if (table->compound()  && table->firstComponentName() != Server::defaultDB) {
        throw Exception("unknown schema/database: " + table->firstComponentName());
    }

    std::string database = table->compound() ? table->firstComponentName() : "test";

    std:: string tablePath = Server::dataPath + database + "/" + table->shortName() + "/";
    if(!std::filesystem::exists(tablePath)) {
        throw Exception("table " + table->shortName() + " does not exist");
    }

    if (columnsList.empty()) {
        throw Exception("empty column list in select expression");
    }


    std::vector<std::ifstream> colFiles;
    std::vector<std::string> rowValues;

    for (auto col: columnsList) {
        if(col->astElem->compound() && col->astElem->firstComponentName() != table->shortName() && col->astElem->firstComponentName() != table->alias ) {
            throw Exception("unknown schema in column '" + col->astElem->name + "'");
        }
        std::string fileName = tablePath + col->astElem->shortName();
        if(!std::filesystem::exists(fileName)) {
            throw Exception("column '" + col->astElem->name + "' does not exist");
        }
        std::ifstream colFile(fileName, std::ofstream::binary);
        colFiles.push_back(std::move(colFile));
    }

    BlockPtr block = std::make_shared<Block>();
    block->columns = columnsList;
    std::list<Column> columns;

    bool stopReading = false;

    while (!stopReading) {

        rowValues.clear();

        for (int i = 0; i < colFiles.size(); ++i) {
            auto &colFile = colFiles[i];
            if (!colFile) {
                stopReading = true;
                rowValues.clear();
                break;
            }

            std::string val;
            if (std::getline (colFile,val)) {
                rowValues.push_back(val);
            }
        }

        for (int i = 0; i < rowValues.size(); ++i) {
            block->columns[i]->data.push_back(rowValues[i]);
        }
    }

    out->push(block);
    out->close();

    return out;
}
