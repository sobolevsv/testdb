#include <iostream>
#include <Processors/FilterStep.h>
#include <Parser/ASTTableExpression.h>
#include <Processors/DropColumnsStep.h>
#include <Processors/GroupByStep.h>
#include <Processors/LimitStep.h>
#include <Parser/ASTOrderByElement.h>
#include "InterpreterSelectQuery.h"
#include "Parser/ASTIdentifier.h"
#include "Parser/ASTFunction.h"
#include "Parser/ASTLiteral.h"
#include "Processors/SelectStep.h"
#include "Processors/OrderByStep.h"


std::ostream &operator<<(std::ostream &os, const ASTIdentifierPtr &as) {
    os << as->getID(':') << "; alias: " << as->alias;

    if (as->compound()) {
        os << "; first part: " << as->firstComponentName() << "; second part: " << as->shortName();
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, const ASTLiteralPtr &as) {
    os << as->getID(':');

    return os;
}

std::ostream &operator<<(std::ostream &os, const ASTFunctionPtr &as) {
    if (as == nullptr)
        return os;
    os << as->getID(':') << std::endl;

    for (auto arg : as->arguments->children) {
        if (std::dynamic_pointer_cast<ASTIdentifier>(arg)) {
            os << "  " << std::dynamic_pointer_cast<ASTIdentifier>(arg);
        } else if (std::dynamic_pointer_cast<ASTLiteral>(arg)) {
            os << "  " << std::dynamic_pointer_cast<ASTLiteral>(arg);
        }
        os << std::endl;
    }
    return os;
}

ASTIdentifierList getColumns(ASTPtr as) {
    ASTIdentifierList columns;
    for (auto a: as->children) {
        auto column = std::dynamic_pointer_cast<ASTIdentifier>(a);
        if (column) {
            columns.push_back(column);
            continue;
        }
    }
    return columns;
}

ASTIdentifierList getGroupByColumns(ASTIdentifierList selectColumn, ASTPtr groupby) {
    ASTIdentifierList columns;
    if (!groupby)
        return columns;
    for (auto a: groupby->children) {
        auto column = std::dynamic_pointer_cast<ASTIdentifier>(a);
        if (column) {
            columns.push_back(column);
            continue;
        }

        // check if column specified by number
        auto literal = std::dynamic_pointer_cast<ASTLiteral>(a);
        if (literal) {
            if (isInt64FieldType(literal->value.getType())) {
                int index = literal->value.get<int>();
                if (index > 0 && index <= selectColumn.size()) {
                    columns.push_back(selectColumn[index - 1]); // 1-based index
                } else {
                    throw Exception("column index '" + std::to_string(index) + "' in ORDER BY is out of range");
                }
            }
            continue;
        }
    }
    return columns;
}

orderByColumnList getOrderByColumns(ASTIdentifierList selectColumn, ASTPtr orderBy) {
    orderByColumnList columns;
    if (!orderBy)
        return columns;

    std::map<std::string, int> selectColNameSet;
    std::map<std::string, int> selectColAliasSet;
    for (int i = 0; i < selectColumn.size(); ++i) {
        auto &col = selectColumn[i];
        selectColNameSet[col->shortName()] = i;
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
                if (index > 0 && index <= selectColumn.size()) {
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

functionList getAgrFunctions(ASTPtr as) {
    functionList functions;
    for (auto a: as->children) {
        auto function = std::dynamic_pointer_cast<ASTFunction>(a);
        if (function) {
            if (function->name != "count") {
                throw Exception("only `count` function supported");
            }
            functions.push_back(function);
        }
    }

    return functions;
}

ASTIdentifierPtr getTables(ASTPtr as) {
    ASTIdentifierPtr table;
    for (auto a: as->children) {
        auto tablesInSelect = std::dynamic_pointer_cast<ASTTablesInSelectQueryElement>(a);
        if (!tablesInSelect) {
            continue;
        }
        auto tableExpr = std::dynamic_pointer_cast<ASTTableExpression>(tablesInSelect->table_expression);
        if (!tableExpr)
            continue;
        table = std::dynamic_pointer_cast<ASTIdentifier>(tableExpr->database_and_table_name);
    }
    return table;
}

ASTFunctionPtr getFilterFunctions(ASTPtr as) {
    auto function = std::dynamic_pointer_cast<ASTFunction>(as);
    if (function && function->name != "equals") {
        throw Exception("only `equals` supported in WHERE");
    }

    return function;
}

ASTIdentifierList getFilterColumnsFunctions(ASTPtr as) {
    ASTIdentifierList columns;
    auto function = std::dynamic_pointer_cast<ASTFunction>(as);
    if (function) {
        for (auto arg : function->arguments->children) {
            auto column = std::dynamic_pointer_cast<ASTIdentifier>(arg);
            if (column) {
                columns.push_back(column);
            }
        }
    }
    return columns;
}

bool combineColumns(ASTIdentifierList selectColumn, ASTIdentifierList whereColumn, ASTIdentifierList &res) {
    bool combined = false;
    std::set<std::string> selectColNameSet;
    std::set<std::string> selectColAliasSet;
    for (auto &a: selectColumn) {
        selectColNameSet.insert(a->shortName());
        if (!a->alias.empty()) {
            selectColAliasSet.insert(a->alias);
        }
        res.push_back(a);
    }
    for (auto &a : whereColumn) {
        if (selectColNameSet.find(a->shortName()) == selectColNameSet.end() &&
            selectColAliasSet.find(a->shortName()) == selectColAliasSet.end()) {
            res.push_back(a);
            combined = true;
        }
    }
    return combined;
}


BlockStreamPtr InterpreterSelectQuery::execute(ASTPtr as) {
    auto selectQuery = std::dynamic_pointer_cast<ASTSelectQuery>(as);
    auto selectColumns = getColumns(selectQuery->select());
    auto agrFunc = getAgrFunctions(selectQuery->select());

    auto where = getFilterFunctions(selectQuery->where());
    auto whereColumns = getFilterColumnsFunctions(selectQuery->where());
    ASTIdentifierList allColumns;
    bool combined = combineColumns(selectColumns, whereColumns, allColumns);

    auto table = getTables(selectQuery->tables());

    auto groupBy = getGroupByColumns(selectColumns, selectQuery->groupBy());

    auto orderBy = selectQuery->orderBy();

    auto limit = selectQuery->limitLength();

    BlockStreamPtr outputStream = SelectStep(allColumns, table);

    if (where) {
        outputStream = FilterStep(outputStream, where);
        if (combined) {
            outputStream = DropColumnsStep(outputStream, selectColumns);
        }
    }

    if (!agrFunc.empty()) {
        outputStream = GroupByStep(outputStream, agrFunc, groupBy);
    }

    if (orderBy) {
        outputStream = OrderByStep(outputStream, getOrderByColumns(selectColumns, orderBy));
    }

    if (limit) {
        outputStream = LimitStep(outputStream, limit);
    }


    return outputStream;
}
