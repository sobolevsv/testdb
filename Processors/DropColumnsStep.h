#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTIdentifier.h>
#include "DataStreams/Block.h"

// drop temporary  columns created for filtering and aggregation
BlockStreamPtr DropTmpColumnsStep(BlockStreamPtr in) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        auto outBlock = std::make_shared<Block>();

        for (auto &a : block->columns) {
            if (!a->tmp) {
                outBlock->columns.push_back(a);
            }
        }

        out->push(outBlock);
    }

    out->close();

    return out;
}