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


/** Element of expression with ASC or DESC,
  *  and possibly with COLLATE.
  */
class ASTOrderByElement : public IAST
{
public:
    int direction; /// 1 for ASC, -1 for DESC
    int nulls_direction; /// Same as direction for NULLS LAST, opposite for NULLS FIRST.
    bool nulls_direction_was_explicitly_specified;

    /** Collation for locale-specific string comparison. If empty, then sorting done by bytes. */
    ASTPtr collation;

    bool with_fill;
    ASTPtr fill_from;
    ASTPtr fill_to;
    ASTPtr fill_step;

    ASTOrderByElement(
            const int direction_, const int nulls_direction_, const bool nulls_direction_was_explicitly_specified_,
            ASTPtr & collation_, const bool with_fill_, ASTPtr & fill_from_, ASTPtr & fill_to_, ASTPtr & fill_step_)
            : direction(direction_)
            , nulls_direction(nulls_direction_)
            , nulls_direction_was_explicitly_specified(nulls_direction_was_explicitly_specified_)
            , collation(collation_)
            , with_fill(with_fill_)
            , fill_from(fill_from_)
            , fill_to(fill_to_)
            , fill_step(fill_step_)
    {
    }

    String getID(char) const override { return "OrderByElement"; }

    ASTPtr clone() const override
    {
        auto clone = std::make_shared<ASTOrderByElement>(*this);
        clone->cloneChildren();
        return clone;
    }

protected:
    //void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;
};