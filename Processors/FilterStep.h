#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTFunction.h>
#include <Parser/ASTIdentifier.h>
#include <Parser/ASTLiteral.h>

BlockPtr FilterBlock(BlockPtr blockIn, ASTFunctionPtr filter) {
    BlockPtr blockOut = cloneBlockWithoutData(blockIn);

    ASTIdentifierPtr filterColumnName;
    std::string filterStrValue;

    if (filter->arguments->children.size() != 2) {
        throw Exception("wrong expression in WHERE ");
    }
    filterColumnName = std::dynamic_pointer_cast<ASTIdentifier>(filter->arguments->children[0]);

    auto secondFuncArg = filter->arguments->children[1];
    if (std::dynamic_pointer_cast<ASTLiteral>(secondFuncArg)) {
        auto filterColumnValue = std::dynamic_pointer_cast<ASTLiteral>(secondFuncArg);
        filterStrValue = filterColumnValue->value.get<std::string>();
    } else if (std::dynamic_pointer_cast<ASTIdentifier>(secondFuncArg)) {
        auto filterColumnValue = std::dynamic_pointer_cast<ASTIdentifier>(secondFuncArg);
        if(filterColumnValue->compound()) {
            throw Exception("wrong expression in WHERE ");
        }
        filterStrValue = filterColumnValue->name;
    }

    int filterColumnIndex = -1;

    for (int i = 0; i < blockIn->columns.size(); ++i) {
        if (blockIn->columns[i]->columnName == filterColumnName->name ||
            blockIn->columns[i]->columnName == filterColumnName->shortName() ||
            blockIn->columns[i]->alias == filterColumnName->name) {
            filterColumnIndex = i;
            break;
        }
    }

    if (filterColumnIndex < 0) {
        throw Exception("unknown column name in WHERE clause");
    }

    std::vector<std::string> rowValues;

    int numRows = blockIn->columns[filterColumnIndex]->data.size();

    for (int i = 0; i < numRows; ++i) {

        if (boost::get<std::string>(blockIn->columns[filterColumnIndex]->data[i]) != filterStrValue) {
            continue;
        }
        for (int j = 0; j < blockIn->columns.size(); ++j) {
            blockOut->columns[j]->data.push_back(blockIn->columns[j]->data[i]);
        }
        for (int i = 0; i < rowValues.size(); ++i) {
            blockOut->columns[i]->data.push_back(rowValues[i]);
        }
    }

    return blockOut;
}

BlockStreamPtr FilterStep(BlockStreamPtr in, ASTFunctionPtr filter) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        out->push(FilterBlock(block, filter));
    }

    out->close();

    return out;
}