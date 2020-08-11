#include  <iostream>
#include "executeQuery.h"
#include "Parser/Lexer.h"
#include "Parser/TokenIterator.h"
#include "Parser/IParser.h"
#include "Parser/ParserSelectQuery.h"
#include "InterpreterBase.h"
//#include "Parser/ASTSelectQuery.h"


//void executeQuery(
//        ReadBuffer & istr,
//        WriteBuffer & ostr,
//        bool allow_into_outfile,
//        Context & context,
//        std::function<void(const String &, const String &, const String &, const String &)> set_result_details) {

BlockStreamPtr executeQuery( const std::string & query){

    BlockStreamPtr out;
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
        out = InterpreterBase(res);
    }
    return out;
//ASTPtr ast;
//Block streams;
//
//std::tie(ast, streams) = executeQueryImpl(begin, end, context, false, QueryProcessingStage::Complete, may_have_tail, &istr);

}