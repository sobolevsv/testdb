#pragma once

#include <string>
#include <fstream>
#include "DataStreams/Block.h"
#include "Parser/ASTIdentifier.h"
#include "Parser/ASTFunction.h"
#include "DataStreams/BlockStream.h"

//for ( auto arg : as->arguments->children ) {
//if (std::dynamic_pointer_cast<ASTIdentifier>(arg)) {
//os << "  " <<  std::dynamic_pointer_cast<ASTIdentifier>(arg) ;
//} else if (std::dynamic_pointer_cast<ASTLiteral>(arg)) {
//os << "  " << std::dynamic_pointer_cast<ASTLiteral>(arg);
//}
//os << std::endl;
//}

std::vector<std::string> data;
std::string columnName;
std::string alias;

ColumnList makeColumnList(std::vector<ASTIdentifierPtr> columnsName) {
    ColumnList res;
    for (auto a: columnsName) {
        auto col = std::make_shared<Column>();
        col->columnName = a->shortName();
        col->alias = a->alias;
        res.push_back(col);
    }
    return res;
}

BlockStreamPtr SelectStep(std::vector<ASTIdentifierPtr> columnsName) {

    BlockStreamPtr out = std::make_shared<BlockStream>();

    std::vector<std::ifstream> colFiles;
    std::vector<std::string> rowValues;

    for (auto a: columnsName) {
        std::string fileName = "data/" + a->shortName();
        std::ifstream colFile(fileName, std::ofstream::binary);
        colFiles.push_back(std::move(colFile));
    }

    BlockPtr block = std::make_shared<Block>();
    block->columns = makeColumnList(columnsName);
    std::list<Column> columns;

    bool stopReading = false;

    while (!stopReading) {

        rowValues.clear();

        for (int i = 0; i < colFiles.size(); ++i) {
            auto &colFile = colFiles[i];
            if (!colFile) {
                stopReading = true;
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
