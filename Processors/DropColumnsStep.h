#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTIdentifier.h>
#include "DataStreams/Block.h"

// drop unneeded columns
BlockStreamPtr DropColumnsStep(BlockStreamPtr in, ASTIdentifierList columns) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    std::set<std::string> outColumnSet;
    for (auto &a: columns) {
        outColumnSet.insert(a->shortName());
    }

    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        auto outBlock = std::make_shared<Block>();

        for (auto &a : block->columns) {
            if (outColumnSet.find(a->columnName) != outColumnSet.end()) {
                outBlock->columns.push_back(a);
            }
        }

        out->push(outBlock);
    }

    out->close();

    return out;
}