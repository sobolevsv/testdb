#pragma once

#include <list>

#include "IParserBase.h"
#include "CommonParsers.h"


/** Consequent pairs of rows: the operator and the corresponding function. For example, "+" -> "plus".
  * The parsing order of the operators is significant.
  */
using Operators_t = const char **;


/** List of elements separated by something. */
class ParserList : public IParserBase
{
public:
    ParserList(ParserPtr && elem_parser_, ParserPtr && separator_parser_, bool allow_empty_ = true, char result_separator_ = ',')
        : elem_parser(std::move(elem_parser_))
        , separator_parser(std::move(separator_parser_))
        , allow_empty(allow_empty_)
        , result_separator(result_separator_)
    {
    }

    template <typename F>
    static bool parseUtil(Pos & pos, Expected & expected, const F & parse_element, IParser & separator_parser_, bool allow_empty_ = true)
    {
        Pos begin = pos;
        if (!parse_element())
        {
            pos = begin;
            return allow_empty_;
        }

        while (true)
        {
            begin = pos;
            if (!separator_parser_.ignore(pos, expected) || !parse_element())
            {
                pos = begin;
                return true;
            }
        }

        return false;
    }

    template <typename F>
    static bool parseUtil(Pos & pos, Expected & expected, const F & parse_element, TokenType separator, bool allow_empty_ = true)
    {
        ParserToken sep_parser{separator};
        return parseUtil(pos, expected, parse_element, sep_parser, allow_empty_);
    }

    template <typename F>
    static bool parseUtil(Pos & pos, Expected & expected, const F & parse_element, bool allow_empty_ = true)
    {
        return parseUtil(pos, expected, parse_element, TokenType::Comma, allow_empty_);
    }

protected:
    const char * getName() const override { return "list of elements"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
private:
    ParserPtr elem_parser;
    ParserPtr separator_parser;
    bool allow_empty;
    char result_separator;
};


/** An expression with an infix binary left-associative operator.
  * For example, a + b - c + d.
  */
class ParserLeftAssociativeBinaryOperatorList : public IParserBase
{
private:
    Operators_t operators;
    ParserPtr first_elem_parser;
    ParserPtr remaining_elem_parser;

public:
    /** `operators_` - allowed operators and their corresponding functions
      */
    ParserLeftAssociativeBinaryOperatorList(Operators_t operators_, ParserPtr && first_elem_parser_)
            : operators(operators_), first_elem_parser(std::move(first_elem_parser_))
    {
    }

    ParserLeftAssociativeBinaryOperatorList(Operators_t operators_, ParserPtr && first_elem_parser_,
                                            ParserPtr && remaining_elem_parser_)
            : operators(operators_), first_elem_parser(std::move(first_elem_parser_)),
              remaining_elem_parser(std::move(remaining_elem_parser_))
    {
    }

protected:
    const char * getName() const override { return "list, delimited by binary operators"; }

    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};

class ParserBetweenExpression : public IParserBase
{
private:
    //ParserConcatExpression elem_parser;

protected:
    const char * getName() const override { return "BETWEEN expression"; }

    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


class ParserComparisonExpression : public IParserBase
{
private:
    static const char * operators[];
    ParserLeftAssociativeBinaryOperatorList operator_parser {operators, std::make_unique<ParserBetweenExpression>()};

protected:
    const char * getName() const  override{ return "comparison expression"; }

    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override
    {
        return operator_parser.parse(pos, node, expected);
    }
};


/** Parser for nullity checking with IS (NOT) NULL.
  */
class ParserNullityChecking : public IParserBase
{
private:
    ParserComparisonExpression elem_parser;

protected:
    const char * getName() const override { return "nullity checking"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


//class ParserLogicalNotExpression : public IParserBase
//{
//private:
//    static const char * operators[];
//    ParserPrefixUnaryOperatorExpression operator_parser {operators, std::make_unique<ParserNullityChecking>()};
//
//protected:
//    const char * getName() const  override{ return "logical-NOT expression"; }
//
//    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override
//    {
//        return operator_parser.parse(pos, node, expected);
//    }
//};


//class ParserLogicalAndExpression : public IParserBase
//{
//private:
//    ParserVariableArityOperatorList operator_parser {"AND", "and", std::make_unique<ParserLogicalNotExpression>()};
//
//protected:
//    const char * getName() const override { return "logical-AND expression"; }
//
//    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override
//    {
//        return operator_parser.parse(pos, node, expected);
//    }
//};


//class ParserLogicalOrExpression : public IParserBase
//{
//private:
//    ParserVariableArityOperatorList operator_parser {"OR", "or", std::make_unique<ParserLogicalAndExpression>()};
//
//protected:
//    const char * getName() const override { return "logical-OR expression"; }
//
//    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override
//    {
//        return operator_parser.parse(pos, node, expected);
//    }
//};






//class ParserExpressionWithOptionalAlias : public IParserBase
//{
//public:
//    ParserExpressionWithOptionalAlias(bool allow_alias_without_as_keyword);
//protected:
//    ParserPtr impl;
//
//    const char * getName() const override { return "expression with optional alias"; }
//
//    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override
//    {
//        return impl->parse(pos, node, expected);
//    }
//};


/** A comma-separated list of expressions, probably empty. */
class ParserExpressionList : public IParserBase
{
public:
    ParserExpressionList(bool allow_alias_without_as_keyword_)
        : allow_alias_without_as_keyword(allow_alias_without_as_keyword_) {}

protected:
    bool allow_alias_without_as_keyword;

    const char * getName() const override { return "list of expressions"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


class ParserNotEmptyExpressionList : public IParserBase
{
public:
    ParserNotEmptyExpressionList(bool allow_alias_without_as_keyword)
        : nested_parser(allow_alias_without_as_keyword) {}
private:
    ParserExpressionList nested_parser;
protected:
    const char * getName() const override { return "not empty list of expressions"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


class ParserOrderByExpressionList : public IParserBase
{
protected:
    const char * getName() const override { return "order by expression"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


/// Parser for key-value pair, where value can be list of pairs.
class ParserKeyValuePair : public IParserBase
{
protected:
    const char * getName() const override { return "key-value pair"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


/// Parser for list of key-value pairs.
class ParserKeyValuePairsList : public IParserBase
{
protected:
    const char * getName() const override { return "list of pairs"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};


class ParserTTLExpressionList : public IParserBase
{
protected:
    const char * getName() const override { return "ttl expression"; }
    bool parseImpl(Pos & pos, ASTPtr & node, Expected & expected) override;
};