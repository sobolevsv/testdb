#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "Block.h"
#include "threadsafe_queue.h"

using BlockStream = threadsafe_queue<BlockPtr>;
using BlockStreamPtr = std::shared_ptr<BlockStream>;
