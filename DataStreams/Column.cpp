
#include <memory>
#include "Column.h"


ColumnPtr cloneColumnWithoutData(ColumnPtr in) {
    auto res = std::make_shared<Column>();
    res->columnName = in->columnName;
    res->alias = in->alias;
    return res;
}
