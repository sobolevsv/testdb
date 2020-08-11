#pragma once

#include "Column.h"
#include <list>
#include <memory>

struct Block
{
    const int BLOCK_SIZE = 10; // сколько строк в колонках блока
    ColumnList columns;
};

using BlockPtr = std::shared_ptr<Block>;

BlockPtr makeNewBlockLike(BlockPtr in);
