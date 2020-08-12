#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTFunction.h>
#include <Parser/ASTLiteral.h>

BlockPtr CutBlockIfNeeded(BlockPtr blockIn, int limit) {

    if (blockIn->columns.empty()) {
        return blockIn;
    }

    if (blockIn->columns[0]->data.size() < limit) {
        return blockIn;
    }

    for (int j = 0; j < blockIn->columns.size(); ++j) {
        blockIn->columns[j]->data.resize(limit);
    }

    return blockIn;
}

BlockStreamPtr LimitStep(BlockStreamPtr in, ASTPtr as) {
    int limit = 0;
    auto literal = std::dynamic_pointer_cast<ASTLiteral>(as);
    if (!literal) {
        return in;
    }

    if (isInt64FieldType(literal->value.getType())) {
        limit = literal->value.get<int>();
    } else {
        return in;
    }

    BlockStreamPtr out = std::make_shared<BlockStream>();


    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        out->push(CutBlockIfNeeded(block, limit));
    }

    out->close();

    return out;
}