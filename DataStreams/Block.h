#pragma once

#include "Column.h"
#include <list>
#include <memory>

struct Block
{
    ColumnList columns;
};

using BlockPtr = std::shared_ptr<Block>;

BlockPtr cloneBlockWithoutData(BlockPtr in);
