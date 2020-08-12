#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTIdentifier.h>
#include <Parser/ASTFunction.h>
#include <map>

struct AggregationResultRow{
    std::vector<Column::fieldType> rowValues;

    using AggrFuncName = std::string;
    std::map<AggrFuncName, int64_t> rowAggrResults;
};


// only count(*) implemented, but count can be used many times in a select expression
// for example:
//      select "Donor State" State, count(*), count(*) as count2 from donors_1  group by State;
// Group By column may be specified by name or index
//      select "Donor State" State, "Donor Zip" Zip, count(*) from donors_1 group by State,Zip;
//      select "Donor State" State, "Donor Zip" Zip, count(*), count(*) as count2 from donors_1 group by 1,Zip;
//      select "Donor State" State, "Donor Zip" Zip, count(*) from donors_1 group by 1,2;

BlockStreamPtr GroupByStep(BlockStreamPtr in, functionList aggrFunctions, ASTIdentifierList groupByColumns) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    std::set<std::string> outColumnSet;

    for (auto &a: groupByColumns) {
        outColumnSet.insert(a->shortName());
    }

    auto agrColumnPtr = std::make_shared<Column>();

    using rowKey = std::string;

    std::unordered_map<rowKey, AggregationResultRow> results;

    BlockPtr lastBlock; // template for output block

    while (*in) {
        auto block = in->pop();
        if (block->columns.empty())
            continue;

        lastBlock = cloneBlockWithoutData(block);

        for (auto &a : block->columns) {
            if (outColumnSet.find(a->columnName) == outColumnSet.end() &&
                outColumnSet.find(a->alias) == outColumnSet.end()) {
                throw Exception(" Column `" + a->columnName + "` is not in GROUP BY");
            }
        }

        int numRows = block->columns[0]->data.size();

        for (int i = 0; i < numRows; ++i) {
            std::string key;
            for (auto &col : block->columns) {
                key += boost::get<std::string>(col->data[i]);
            }
            for (int j = 0; j < aggrFunctions.size(); ++j) {
                auto it = results.find(key);
                if (it != results.end()) {
                    for( auto &functionResults : it->second.rowAggrResults) {
                        if(functionResults.first == "count") {
                            functionResults.second++;
                        }
                    }
                } else {
                    AggregationResultRow functionsResults;
                    std::vector<std::string> columnValues;
                    for (auto &col : block->columns) {
                        functionsResults.rowValues.push_back(col->data[i]);
                    }
                    for (auto & f : aggrFunctions) {
                        if(f->name == "count") {
                            functionsResults.rowAggrResults[f->name]++;
                        }
                    }
                    results[key] = functionsResults;
                }
            }
        }
    }

    for (int j = 0; j < aggrFunctions.size(); ++j) {
        auto column = std::make_shared<Column>();
        column->columnName = aggrFunctions[j]->name;
        column->alias = aggrFunctions[j]->alias;
        lastBlock->columns.push_back(column);
    }

    for (auto &res : results) {
        for (int i = 0; i < res.second.rowValues.size(); ++i) {
            lastBlock->columns[i]->data.push_back(res.second.rowValues[i]);
        }
        for (int i = 0; i < aggrFunctions.size(); ++i) {
            auto funcRes = res.second.rowAggrResults[aggrFunctions[i]->name];
            lastBlock->columns[res.second.rowValues.size() + i]->data.push_back(funcRes);
        }
    }

    out->push(lastBlock);
    out->close();

    return out;
}
