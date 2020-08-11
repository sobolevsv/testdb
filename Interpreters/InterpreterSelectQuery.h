#pragma once
#include <memory>
#include <DataStreams/BlockStream.h>
#include "Parser/IAST_fwd.h"
#include "Parser/TypePromotion.h"
#include "Parser/ASTSelectQuery.h"


class InterpreterSelectQuery {
public:
    BlockStreamPtr execute(ASTPtr as);

};

