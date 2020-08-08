#include  <iostream>
#include "executeQuery.h"
#include "../Parser/Lexer.h"
#include "../Parser/TokenIterator.h"
#include "../Parser/IParser.h"


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
    IParser::Pos token_iterator(tokens, 10);

//ASTPtr ast;
//BlockIO streams;
//
//std::tie(ast, streams) = executeQueryImpl(begin, end, context, false, QueryProcessingStage::Complete, may_have_tail, &istr);

}