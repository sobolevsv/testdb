#pragma once
#include <vector>
#include <string>
#include <list>
#include "boost/variant.hpp"


struct Column {

    std::string columnName;
    std::string alias;
    using fieldType = boost::variant<int, std::string>;
    std::vector<fieldType> data;

};

using ColumnPtr = std::shared_ptr<Column>;
using ColumnList = std::vector<ColumnPtr>;

ColumnPtr cloneColumnWithoutData(ColumnPtr in);
