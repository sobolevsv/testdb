#include <iostream>
#include <Processors/FilterStep.h>
#include "InterpreterSelectQuery.h"
#include "Parser/ASTIdentifier.h"
#include "Parser/ASTFunction.h"
#include "Parser/ASTLiteral.h"
#include "Processors/SelectStep.h"

using columns_vector = std::vector<ASTIdentifierPtr>;

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


columns_vector traverseSelectExpr(ASTPtr as) {
    columns_vector columns;
    for (auto a: as->children) {
        auto column = std::dynamic_pointer_cast<ASTIdentifier>(a);
        if (column) {
            columns.push_back(column);
        }
    }
    return columns;
}

ASTFunctionPtr traverseWhereExpr(ASTPtr as) {
    auto function = std::dynamic_pointer_cast<ASTFunction>(as);
    if (function && function->name != "equals") {
        throw Exception("only `equals` function supported");
    }

    return function;
}


BlockStreamPtr InterpreterSelectQuery::execute(ASTPtr as)
{
    auto selectQuery = std::dynamic_pointer_cast<ASTSelectQuery>(as);
    auto columns = traverseSelectExpr(selectQuery->select());
    //std::cout << "columns: " << std::endl;
    for(auto &col : columns) {
        std::cout << col << std::endl;
    }

    auto where = traverseWhereExpr(selectQuery->where());
    //std::cout << "where : " << where << std::endl;

    BlockStreamPtr selectout = SelectStep(columns);
    BlockStreamPtr filterOut = FilterStep(selectout, where);

    return filterOut;
}
