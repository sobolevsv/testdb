#pragma once
#include <vector>
#include <string>
#include <list>

struct Column {

    std::string columnName;
    std::string alias;
    std::vector<std::string> data;
};

using ColumnPtr = std::shared_ptr<Column>;
using ColumnList = std::vector<ColumnPtr>;
