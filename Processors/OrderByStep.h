#pragma once

#include <Parser/ASTIdentifier.h>
#include <DataStreams/BlockStream.h>

struct orderByColumn{
    int columnIndex;
    int direction;
};

using orderByColumnList = std::vector<orderByColumn>;

BlockPtr sortBlock(BlockPtr blockIn, orderByColumnList orderByColumns) {

    if (blockIn->columns.empty()) {
        return blockIn;
    }

    std::vector<int> indexes(blockIn->columns[0]->data.size());
    std::iota(indexes.begin(), indexes.end(), 0);

    std::sort(indexes.begin(), indexes.end(), [&orderByColumns, &blockIn](int idx1, int idx2) {
        for (auto & col : orderByColumns) {
            auto & curCol = blockIn->columns[col.columnIndex];
            auto value1 = curCol->data[idx1];
            auto value2 = curCol->data[idx2];
            if (value1 == value2)
                continue;

            return col.direction > 0 ? value1 < value2 : value1 > value2;
        }
        return true;
    });

    for (int j = 0; j < blockIn->columns.size(); ++j) {
        auto & curCol = blockIn->columns[j];
        auto newCol = cloneColumnWithoutData(curCol);
        for (int i = 0; i < indexes.size(); ++i) {
            newCol->data.push_back(curCol->data[indexes[i]]);
        }
        blockIn->columns[j] = newCol;
    }

    return blockIn;
}


BlockStreamPtr OrderByStep(BlockStreamPtr in,  orderByColumnList orderByColumns) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        block = sortBlock(block, orderByColumns);

        out->push(block);
    }

    out->close();

    return out;
}