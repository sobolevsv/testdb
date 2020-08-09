cmake_minimum_required(VERSION 3.17)
project(columndb)

set(CMAKE_CXX_STANDARD 17)

add_executable(columndb main.cpp Server/Server.cpp Server/Server.h Server/csvparser.h Parser/Lexer.cpp Parser/Lexer.h Parser/find_symbols.h
        Parser/StringUtils.cpp Parser/StringUtils.h Interpreters/executeQuery.cpp Interpreters/executeQuery.h
        Parser/TokenIterator.cpp Parser/TokenIterator.h Parser/Exception.h
        Parser/IAST.cpp Parser/IAST.h Parser/IAST_fwd.h Parser/IParserBase.cpp Parser/IParserBase.h
        Parser/CommonParsers.cpp Parser/CommonParsers.h
        Parser/ParserSelectQuery.cpp  Parser/ParserSelectQuery.h
        Parser/ASTSelectQuery.cpp Parser/ASTSelectQuery.h
        Parser/Names.h
        Parser/ASTTableExpression.cpp Parser/ASTTableExpression.h
        Parser/TypePromotion.h Parser/typeid_cast.h
        Parser/demangle.cpp Parser/demangle.h
        Parser/shared_ptr_helper.h
        Parser/ExpressionListParsers.cpp
        Parser/ExpressionListParsers.h
        Parser/ExpressionElementParsers.cpp
        Parser/ExpressionElementParsers.h
        Parser/ASTExpressionList.h
        Parser/ASTWithAlias.cpp
        Parser/ASTOrderByElement.h
        Parser/ASTFunction.cpp
        Parser/ASTFunction.h
        Parser/ASTIdentifier.cpp
        Parser/ASTIdentifier.h
        Parser/IdentifierSemantic.cpp
        Parser/IdentifierSemantic.h
        Parser/ParserTablesInSelectQuery.cpp
        Parser/ParserTablesInSelectQuery.h
        Parser/ASTTablesInSelectQuery.cpp
        Parser/ASTTablesInSelectQuery.h
        Parser/ASTLiteral.cpp
        Parser/ASTLiteral.h
        Parser/types.h
        Parser/Types.h
        Parser/intExp.h
        Parser/Field.cpp
        Parser/Field.h
        Parser/defines.h
        Parser/Defines.h
        )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(columndb PRIVATE Threads::Threads)
