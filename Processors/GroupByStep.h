#pragma once

#include <DataStreams/BlockStream.h>
#include <Parser/ASTIdentifier.h>
#include <Parser/ASTFunction.h>
#include <map>

struct AggregationResultRow{
    std::vector<std::string> columnValues;

    using AggrFuncName = std::string;
    std::map<AggrFuncName, int64_t> functionsResults;
};

// only count(*) implemented, but could be many count(*)
BlockStreamPtr GroupByStep(BlockStreamPtr in, functionList aggrFunctions, ASTIdentifierList groupByColumns) {
    BlockStreamPtr out = std::make_shared<BlockStream>();

    std::set<std::string> outColumnSet;
    for (auto &a: groupByColumns) {
        outColumnSet.insert(a->shortName());
    }

    auto agrColumnPtr = std::make_shared<Column>();

    //using RowValues = std::vector<std::string>;
    //using AggrValue = int64_t;

//    using AggrFunctionsResults = std::map<AggrFuncName, AggrValue>;
    using rowKey = std::string;

    std::unordered_map<rowKey, AggregationResultRow> results;

    //std::vector<aggrFunctionResult> aggrResults(aggrFunctions.size());

    BlockPtr lastBlock; // for template

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
                key += col->data[i];
            }
            for (int j = 0; j < aggrFunctions.size(); ++j) {
                auto it = results.find(key);
                if (it != results.end()) {
                    for( auto &functionResults : it->second.functionsResults) {
                        if(functionResults.first == "count") {
                            functionResults.second++;
                        }
                    }
                } else {
                    AggregationResultRow functionsResults;
                    std::vector<std::string> columnValues;
                    for (auto &col : block->columns) {
                        functionsResults.columnValues.push_back(col->data[i]);
                    }
                    for (auto & f : aggrFunctions) {
                        if(f->name == "count") {
                            functionsResults.functionsResults[f->name]++;
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
        for (int i = 0; i < res.second.columnValues.size(); ++i) {
            lastBlock->columns[i]->data.push_back(res.second.columnValues[i]);
        }
        for (int i = 0; i < aggrFunctions.size(); ++i) {
            //lastBlock->columns[i]->data.push_back(res.second.columnValues[i]);
            auto funcRes = res.second.functionsResults[aggrFunctions[i]->name];
            lastBlock->columns[res.second.columnValues.size() + i]->data.push_back( std::to_string(funcRes));
            //lastBlock->columns[res.second.second.size() + i]->data.push_back(res.second.first[""]);
        }
    }

    out->push(lastBlock);
    out->close();

    return out;

    return out;
}
