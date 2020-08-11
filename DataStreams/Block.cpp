
#include <memory>
#include "Block.h"


BlockPtr makeNewBlockLike(BlockPtr in)  {
    auto res = std::make_shared<Block>();
    for (const auto & curCol: in->columns) {
        res->columns.push_back(cloneWithoutData(curCol));
    }
    return res;
}
