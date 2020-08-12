#pragma once

#include <Parser/ASTIdentifier.h>
#include <DataStreams/BlockStream.h>

struct orderByColumn{
    int columnIndex;
    int direction;
};

using orderByColumnList = std::vector<orderByColumn>;


orderByColumnList getOrderByColumns(BlockPtr blockIn, ASTPtr orderBy) {
    orderByColumnList columns;
    if (!orderBy)
        return columns;

    std::map<std::string, int> selectColNameSet;
    std::map<std::string, int> selectColAliasSet;
    for (int i = 0; i < blockIn->columns.size(); ++i) {
        auto &col = blockIn->columns[i];
        selectColNameSet[col->columnName] = i;
        if (!col->alias.empty()) {
            selectColAliasSet[col->alias] = i;
        }
    }

    for (auto a: orderBy->children) {
        auto orderByEl = std::dynamic_pointer_cast<ASTOrderByElement>(a);
        if (!orderByEl) {
            continue;
        }

        if (orderByEl->children.empty()) {
            continue;
        }

        auto orderByLiteral = std::dynamic_pointer_cast<ASTLiteral>(orderByEl->children[0]);
        if (orderByLiteral) {
            orderByColumn col;
            col.direction = orderByEl->direction;

            if (isInt64FieldType(orderByLiteral->value.getType())) {
                int index = orderByLiteral->value.get<int>();
                if (index > 0 && index <= blockIn->columns.size()) {
                    col.columnIndex = index - 1; // 1-based index
                    columns.push_back(col);
                } else {
                    throw Exception("column index '" + std::to_string(index) + "' in ORDER BY is out of range");
                }
            }
            continue;
        }
        auto orderByColumnName = std::dynamic_pointer_cast<ASTIdentifier>(orderByEl->children[0]);
        if (orderByColumnName) {
            orderByColumn col;
            col.direction = orderByEl->direction;
            auto indexByName = selectColNameSet.find(orderByColumnName->shortName());
            if (indexByName != selectColNameSet.end()) {
                col.columnIndex = indexByName->second;
                columns.push_back(col);
                continue;
            }

            auto indexByAlias = selectColAliasSet.find(orderByColumnName->shortName());
            if (indexByAlias != selectColAliasSet.end()) {
                col.columnIndex = indexByAlias->second;
                columns.push_back(col);
                continue;
            }
            throw Exception("unknown column name '" + orderByColumnName->shortName() + "' in ORDER BY");
        }
    }
    return columns;
}


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
        return false;
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


BlockStreamPtr OrderByStep(BlockStreamPtr in,  ASTPtr orderBy) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        auto orderByColumns = getOrderByColumns(block, orderBy);
        block = sortBlock(block, orderByColumns);

        out->push(block);
    }

    out->close();

    return out;
}