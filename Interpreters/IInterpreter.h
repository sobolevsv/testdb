#pragma once

//#include <DataStreams/Block.h>

struct Block{

};


/** Interpreters interface for different queries.
  */
class IInterpreter
{
public:
    /** For queries that return a result (SELECT and similar), sets in Block a stream from which you can read this result.
      * For queries that receive data (INSERT), sets a thread in Block where you can write data.
      * For queries that do not require data and return nothing, Block will be empty.
      */
    virtual Block execute() = 0;

    virtual bool ignoreQuota() const { return false; }
    virtual bool ignoreLimits() const { return false; }

    virtual ~IInterpreter() = default;
};
