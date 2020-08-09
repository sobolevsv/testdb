#include "typeid_cast.h"
//#include <Parsers/ASTSetQuery.h>

//#include <Parsers/ASTFunction.h>
//#include <Parsers/ASTIdentifier.h>

//#include "ASTSelectQuery.h"
#include "ASTOrderByElement.h"
//#include <Parsers/ASTTablesInSelectQuery.h>

//#include <Interpreters/StorageID.h>

#include "ASTTableExpression.h"
#include "ASTSelectQuery.h"


namespace ErrorCodes
{
    extern const int NOT_IMPLEMENTED;
    extern const int LOGICAL_ERROR;
}


ASTPtr ASTSelectQuery::clone() const
{
    auto res = std::make_shared<ASTSelectQuery>(*this);
    res->children.clear();
    res->positions.clear();

#define CLONE(expr) res->setExpression(expr, getExpression(expr, true))

    /** NOTE Members must clone exactly in the same order,
        *  in which they were inserted into `children` in ParserSelectQuery.
        * This is important because of the children's names the identifier (getTreeHash) is compiled,
        *  which can be used for column identifiers in the case of subqueries in the IN statement.
        * For distributed query processing, in case one of the servers is localhost and the other one is not,
        *  localhost query is executed within the process and is cloned,
        *  and the request is sent to the remote server in text form via TCP.
        * And if the cloning order does not match the parsing order,
        *  then different servers will get different identifiers.
        */
    CLONE(Expression::WITH);
    CLONE(Expression::SELECT);
    CLONE(Expression::TABLES);
    CLONE(Expression::PREWHERE);
    CLONE(Expression::WHERE);
    CLONE(Expression::GROUP_BY);
    CLONE(Expression::HAVING);
    CLONE(Expression::ORDER_BY);
    CLONE(Expression::LIMIT_BY_OFFSET);
    CLONE(Expression::LIMIT_BY_LENGTH);
    CLONE(Expression::LIMIT_BY);
    CLONE(Expression::LIMIT_OFFSET);
    CLONE(Expression::LIMIT_LENGTH);
    CLONE(Expression::SETTINGS);

#undef CLONE

    return res;
}





static const ASTTableExpression * getFirstTableExpression(const ASTSelectQuery & select)
{
    if (!select.tables())
        return {};

    const auto & tables_in_select_query = select.tables()->as<ASTTablesInSelectQuery &>();
    if (tables_in_select_query.children.empty())
        return {};

    const auto & tables_element = tables_in_select_query.children[0]->as<ASTTablesInSelectQueryElement &>();
    if (!tables_element.table_expression)
        return {};

    return tables_element.table_expression->as<ASTTableExpression>();
}

static ASTTableExpression * getFirstTableExpression(ASTSelectQuery & select)
{
    if (!select.tables())
        return {};

    auto & tables_in_select_query = select.tables()->as<ASTTablesInSelectQuery &>();
    if (tables_in_select_query.children.empty())
        return {};

    auto & tables_element = tables_in_select_query.children[0]->as<ASTTablesInSelectQueryElement &>();
    if (!tables_element.table_expression)
        return {};

    return tables_element.table_expression->as<ASTTableExpression>();
}


static const ASTTablesInSelectQueryElement * getFirstTableJoin(const ASTSelectQuery & select)
{
    if (!select.tables())
        return nullptr;

    const auto & tables_in_select_query = select.tables()->as<ASTTablesInSelectQuery &>();
    if (tables_in_select_query.children.empty())
        return nullptr;

    const ASTTablesInSelectQueryElement * joined_table = nullptr;
    for (const auto & child : tables_in_select_query.children)
    {
        const auto & tables_element = child->as<ASTTablesInSelectQueryElement &>();
        if (tables_element.table_join)
        {
            if (!joined_table)
                joined_table = &tables_element;
            else
                throw Exception("Multiple JOIN does not support the query.");
        }
    }

    return joined_table;
}


ASTPtr ASTSelectQuery::sampleSize() const
{
    const ASTTableExpression * table_expression = getFirstTableExpression(*this);
    if (!table_expression)
        return {};

    return table_expression->sample_size;
}


ASTPtr ASTSelectQuery::sampleOffset() const
{
    const ASTTableExpression * table_expression = getFirstTableExpression(*this);
    if (!table_expression)
        return {};

    return table_expression->sample_offset;
}


bool ASTSelectQuery::final() const
{
    const ASTTableExpression * table_expression = getFirstTableExpression(*this);
    if (!table_expression)
        return {};

    return table_expression->final;
}

bool ASTSelectQuery::withFill() const
{
    if (!orderBy())
        return false;

    for (const auto & order_expression_element : orderBy()->children)
        if (order_expression_element->as<ASTOrderByElement &>().with_fill)
            return true;

    return false;
}



const ASTTablesInSelectQueryElement * ASTSelectQuery::join() const
{
    return getFirstTableJoin(*this);
}

//static String getTableExpressionAlias(const ASTTableExpression * table_expression)
//{
//    if (table_expression->subquery)
//        return table_expression->subquery->tryGetAlias();
//    else if (table_expression->table_function)
//        return table_expression->table_function->tryGetAlias();
//    else if (table_expression->database_and_table_name)
//        return table_expression->database_and_table_name->tryGetAlias();
//
//    return String();
//}

//void ASTSelectQuery::replaceDatabaseAndTable(const String & database_name, const String & table_name)
//{
//    assert(database_name != "_temporary_and_external_tables");
//    replaceDatabaseAndTable(StorageID(database_name, table_name));
//}

//void ASTSelectQuery::replaceDatabaseAndTable(const StorageID & table_id)
//{
//    ASTTableExpression * table_expression = getFirstTableExpression(*this);
//
//    if (!table_expression)
//    {
//        setExpression(Expression::TABLES, std::make_shared<ASTTablesInSelectQuery>());
//        auto element = std::make_shared<ASTTablesInSelectQueryElement>();
//        auto table_expr = std::make_shared<ASTTableExpression>();
//        element->table_expression = table_expr;
//        element->children.emplace_back(table_expr);
//        tables()->children.emplace_back(element);
//        table_expression = table_expr.get();
//    }
//
//    String table_alias = getTableExpressionAlias(table_expression);
//    table_expression->database_and_table_name = createTableIdentifier(table_id);
//
//    if (!table_alias.empty())
//        table_expression->database_and_table_name->setAlias(table_alias);
//}


//void ASTSelectQuery::addTableFunction(ASTPtr & table_function_ptr)
//{
//    ASTTableExpression * table_expression = getFirstTableExpression(*this);
//
//    if (!table_expression)
//    {
//        setExpression(Expression::TABLES, std::make_shared<ASTTablesInSelectQuery>());
//        auto element = std::make_shared<ASTTablesInSelectQueryElement>();
//        auto table_expr = std::make_shared<ASTTableExpression>();
//        element->table_expression = table_expr;
//        element->children.emplace_back(table_expr);
//        tables()->children.emplace_back(element);
//        table_expression = table_expr.get();
//    }
//
//    String table_alias = getTableExpressionAlias(table_expression);
//    /// Maybe need to modify the alias, so we should clone new table_function node
//    table_expression->table_function = table_function_ptr->clone();
//    table_expression->database_and_table_name = nullptr;
//
//    if (table_alias.empty())
//        table_expression->table_function->setAlias(table_alias);
//}

void ASTSelectQuery::setExpression(Expression expr, ASTPtr && ast)
{
    if (ast)
    {
        auto it = positions.find(expr);
        if (it == positions.end())
        {
            positions[expr] = children.size();
            children.emplace_back(ast);
        }
        else
            children[it->second] = ast;
    }
    else if (positions.count(expr))
    {
        size_t pos = positions[expr];
        children.erase(children.begin() + pos);
        positions.erase(expr);
        for (auto & pr : positions)
            if (pr.second > pos)
                --pr.second;
    }
}

ASTPtr & ASTSelectQuery::getExpression(Expression expr)
{
    if (!positions.count(expr))
        throw Exception("Get expression before set");
    return children[positions[expr]];
}
