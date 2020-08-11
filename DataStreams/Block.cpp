
#include <memory>
#include "Block.h"


BlockPtr cloneBlockWithoutData(BlockPtr in)  {
    auto res = std::make_shared<Block>();
    for (const auto & curCol: in->columns) {
        res->columns.push_back(cloneColumnWithoutData(curCol));
    }
    return res;
}
