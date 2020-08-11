#include <iostream>
#include <Processors/FilterStep.h>
#include <Parser/ASTTableExpression.h>
#include "InterpreterSelectQuery.h"
#include "Parser/ASTIdentifier.h"
#include "Parser/ASTFunction.h"
#include "Parser/ASTLiteral.h"
#include "Processors/SelectStep.h"

using ASTIdentifierList = std::vector<ASTIdentifierPtr>;
using functionList = std::vector<ASTFunctionPtr>;

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

    for ( auto arg : as->arguments->children ) {
        if (std::dynamic_pointer_cast<ASTIdentifier>(arg)) {
            os << "  " <<  std::dynamic_pointer_cast<ASTIdentifier>(arg) ;
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
        }
    }
    return columns;
}

functionList getAgrFunctions(ASTPtr as) {
    functionList functions;
    for (auto a: as->children) {
        auto function = std::dynamic_pointer_cast<ASTFunction>(a);
        if (function && function->name != "count") {
            throw Exception("only `count` function supported");
        }
        functions.push_back(function);
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
        if(!tableExpr)
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


BlockStreamPtr InterpreterSelectQuery::execute(ASTPtr as)
{
    auto selectQuery = std::dynamic_pointer_cast<ASTSelectQuery>(as);
    auto columns = getColumns(selectQuery->select());
    auto agrFunc = getAgrFunctions(selectQuery->select());

    for(auto &col : columns) {
        std::cout << col << std::endl;
    }

    auto where = getFilterFunctions(selectQuery->where());

    auto table = getTables(selectQuery->tables());
    auto groupBy = selectQuery->groupBy();

    BlockStreamPtr selectOut = SelectStep(columns, table);
    BlockStreamPtr filterOut = selectOut;
    if (where) {
        filterOut = FilterStep(selectOut, where);
    }

    return filterOut;
}
