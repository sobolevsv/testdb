#pragma once

#include "IAST.h"

struct ASTTableExpression : public IAST {
    /// One of fields is non-nullptr.
    ASTPtr database_and_table_name;
    ASTPtr table_function;
    ASTPtr subquery;

    /// Modifiers
    bool final = false;
    ASTPtr sample_size;
    ASTPtr sample_offset;

    using IAST::IAST;
    String getID(char) const override { return "TableExpression"; }
    //ASTPtr clone() const override;
    //void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;
};

/// The list. Elements are in 'children' field.
struct ASTTablesInSelectQuery : public IAST
{
    using IAST::IAST;
    String getID(char) const override { return "TablesInSelectQuery"; }
    //ASTPtr clone() const override;
    //void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;
};

/// Element of list.
struct ASTTablesInSelectQueryElement : public IAST
{
    /** For first element of list, either table_expression or array_join element could be non-nullptr.
      * For former elements, either table_join and table_expression are both non-nullptr, or array_join is non-nullptr.
      */
    ASTPtr table_join;       /// How to JOIN a table, if table_expression is non-nullptr.
    ASTPtr table_expression; /// Table.
    ASTPtr array_join;       /// Arrays to JOIN.

    using IAST::IAST;
    String getID(char) const override { return "TablesInSelectQueryElement"; }
    //ASTPtr clone() const override;
    //void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;
};

