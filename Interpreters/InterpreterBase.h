#pragma once

#include "Parser/IAST_fwd.h"
#include "Parser/TypePromotion.h"
#include "Parser/ASTSelectQuery.h"
#include "InterpreterSelectQuery.h"



BlockStreamPtr InterpreterBase(ASTPtr & query) {
    BlockStreamPtr res;
    if (query->as<ASTSelectQuery>()) {
        InterpreterSelectQuery selectQuery;
        res = selectQuery.execute(query);
    }

    return res;
}
