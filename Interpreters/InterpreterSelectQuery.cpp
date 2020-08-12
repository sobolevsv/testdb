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

ColumnList getColumns(ASTPtr as) {
    ColumnList columns;
    for (auto a: as->children) {
        auto columnIdent = std::dynamic_pointer_cast<ASTIdentifier>(a);
        if (columnIdent) {
            auto columnPtr = makeColumnFromASTColumn(columnIdent);
            columns.push_back(columnPtr);
            continue;
        }
    }
    return columns;
}

ColumnList getGroupByColumns(ColumnList selectColumn, ASTPtr groupby) {
    ColumnList columns;
    if (!groupby)
        return columns;
    for (auto a: groupby->children) {
        auto astColumn = std::dynamic_pointer_cast<ASTIdentifier>(a);
        if (astColumn) {
            auto columnPtr = makeColumnFromASTColumn(astColumn);
            columns.push_back(columnPtr);
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

ColumnList getFilterColumnsFunctions(ASTPtr as) {
    ColumnList columns;
    auto function = std::dynamic_pointer_cast<ASTFunction>(as);
    if (function) {
        if (function->arguments->children.size() != 2) {
            throw Exception("wrong expression in WHERE ");
        }
        auto ASTColumn = std::dynamic_pointer_cast<ASTIdentifier>(function->arguments->children[0]);
        if (ASTColumn) {
            auto columnPtr = makeColumnFromASTColumn(ASTColumn);
            columns.push_back(columnPtr);
        }
    }
    return columns;
}

ColumnList combineColumns(ColumnList selectColumn, ColumnList whereColumn) {
    ColumnList res;
    std::set<std::string> selectColNameSet;
    std::set<std::string> selectColAliasSet;
    for (auto &a: selectColumn) {
        selectColNameSet.insert(a->columnName);
        if (!a->alias.empty()) {
            selectColAliasSet.insert(a->alias);
        }
        res.push_back(a);
    }
    for (auto &a : whereColumn) {
        if (selectColNameSet.find(a->columnName) == selectColNameSet.end() &&
            selectColAliasSet.find(a->columnName) == selectColAliasSet.end()) {
            a->tmp = true;
            res.push_back(a);
        }
    }
    return res;
}


BlockStreamPtr InterpreterSelectQuery::execute(ASTPtr as) {
    auto selectQuery = std::dynamic_pointer_cast<ASTSelectQuery>(as);
    auto selectColumns = getColumns(selectQuery->select());
    auto agrFunc = getAgrFunctions(selectQuery->select());

    auto where = getFilterFunctions(selectQuery->where());
    auto whereColumns = getFilterColumnsFunctions(where);
    ColumnList allColumns =  combineColumns(selectColumns, whereColumns);

    auto table = getTables(selectQuery->tables());

    auto groupBy = getGroupByColumns(selectColumns, selectQuery->groupBy());

    auto orderBy = selectQuery->orderBy();

    auto limit = selectQuery->limitLength();

    BlockStreamPtr outputStream = SelectStep(allColumns, table);

    if (where) {
        outputStream = FilterStep(outputStream, where);
    }

    if (!agrFunc.empty()) {
        outputStream = GroupByStep(outputStream, agrFunc, groupBy);
    }

    if (where || !agrFunc.empty()) {
        outputStream = DropTmpColumnsStep(outputStream);
    }

    if (orderBy) {
        outputStream = OrderByStep(outputStream, orderBy);
    }

    if (limit) {
        outputStream = LimitStep(outputStream, limit);
    }


    return outputStream;
}
