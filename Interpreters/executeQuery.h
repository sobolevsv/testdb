#pragma once

#include <string>
#include <DataStreams/BlockStream.h>

BlockStreamPtr executeQuery( const std::string & query);
