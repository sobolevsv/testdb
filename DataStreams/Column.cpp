
#include <memory>
#include "Column.h"


ColumnPtr cloneColumnWithoutData(ColumnPtr in) {
    auto res = std::make_shared<Column>();
    res->columnName = in->columnName;
    res->alias = in->alias;
    res->tmp = in->tmp;
    return res;
}

ColumnPtr makeColumnFromASTColumn(ASTIdentifierPtr in) {
    auto columnPtr = std::make_shared<Column>();
    columnPtr->columnName = in->shortName();
    columnPtr->alias = in->alias;
    columnPtr->astElem = in;
    return columnPtr;
}
