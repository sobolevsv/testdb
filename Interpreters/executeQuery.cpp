#include  <iostream>
#include "executeQuery.h"
#include "Parser/Lexer.h"
#include "Parser/TokenIterator.h"
#include "Parser/IParser.h"
#include "Parser/ParserSelectQuery.h"
#include "InterpreterBase.h"

BlockStreamPtr executeQuery( const std::string & query){

    auto begin = query.c_str();
    auto end = query.c_str() + query.size();

    Tokens tokens(begin, end, 1000);
    IParser::Pos token_iterator(tokens, 100);

    Expected expected;
    ASTPtr res;

    try {
        ParserSelectQuery parser;
        parser.parse(token_iterator, res, expected);
    } catch( const Exception & e ) {
        std::cout << "field to parse request: " << e.what();
    }

    if (res) {
        return InterpreterBase(res);
    }

    throw Exception("failed to parse query: " + query);
}
