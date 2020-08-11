#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTFunction.h>
#include <Parser/ASTIdentifier.h>
#include <Parser/ASTLiteral.h>

BlockPtr FilterBlock(BlockPtr blockIn, ASTFunctionPtr filter ){
    BlockPtr blockOut = cloneBlockWithoutData(blockIn);

    ASTIdentifierPtr filterColumnName;
    ASTLiteralPtr filterColumnValue;
    std::string filterStrValue;

    for ( auto arg : filter->arguments->children ) {
        if (std::dynamic_pointer_cast<ASTIdentifier>(arg)) {
            filterColumnName = std::dynamic_pointer_cast<ASTIdentifier>(arg) ;
        } else if (std::dynamic_pointer_cast<ASTLiteral>(arg)) {
            filterColumnValue = std::dynamic_pointer_cast<ASTLiteral>(arg);
            filterStrValue = filterColumnValue->value.get<std::string>();
        }
    }

    int filterColumnIndex = -1;

    for (int i = 0; i < blockIn->columns.size(); ++i) {
        if (blockIn->columns[i]->columnName == filterColumnName->name ||
                blockIn->columns[i]->alias == filterColumnName->name) {
            filterColumnIndex = i;
            break;
        }
    }

    if (filterColumnIndex < 0 ) {
        throw Exception("unknown column name in WHERE clause");
    }

    std::vector<std::string> rowValues;

    int numRows = blockIn->columns[filterColumnIndex]->data.size();

    for (int i = 0; i < numRows; ++i) {

        if(blockIn->columns[filterColumnIndex]->data[i] != filterStrValue){
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

BlockStreamPtr FilterStep(BlockStreamPtr in, ASTFunctionPtr filter ) {
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