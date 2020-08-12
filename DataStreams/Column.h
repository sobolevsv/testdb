#pragma once
#include <vector>
#include <string>
#include <list>
#include <Parser/ASTIdentifier.h>
#include "boost/variant.hpp"


struct Column {

    std::string columnName;
    std::string alias;
    using fieldType = boost::variant<int, std::string>;
    std::vector<fieldType> data;

    ASTIdentifierPtr astElem;
    bool tmp = false; // temporary column for filtering or aggregation
};

using ColumnPtr = std::shared_ptr<Column>;
using ColumnList = std::vector<ColumnPtr>;

ColumnPtr cloneColumnWithoutData(ColumnPtr in);
ColumnPtr makeColumnFromASTColumn(ASTIdentifierPtr in);
