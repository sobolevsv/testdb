#include  <iostream>
#include "executeQuery.h"
#include "../Parser/Lexer.h"
#include "../Parser/TokenIterator.h"
#include "../Parser/IParser.h"
#include "../Parser/ParserSelectQuery.h"
#include "../Parser/Exception.h"
#include "../Parser/ASTSelectQuery.h"


//void executeQuery(
//        ReadBuffer & istr,
//        WriteBuffer & ostr,
//        bool allow_into_outfile,
//        Context & context,
//        std::function<void(const String &, const String &, const String &, const String &)> set_result_details) {

void executeQuery( const std::string & query){

    auto begin = query.c_str();
    auto end = query.c_str() + query.size();

    Lexer lexer(begin, end, 1000);

    auto token = lexer.nextToken();

    while(!token.isEnd()) {
        std::cout << getTokenName(token.type) << '\n';
        token = lexer.nextToken();
    }

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
        const ASTSelectQuery & select = res->as<ASTSelectQuery &>();

        std::cout << "query parsed: " << res->getColumnName();

        auto where =  select.where();
    }


//ASTPtr ast;
//BlockIO streams;
//
//std::tie(ast, streams) = executeQueryImpl(begin, end, context, false, QueryProcessingStage::Complete, may_have_tail, &istr);

}