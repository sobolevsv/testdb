#pragma once

#include <string>
#include <stdexcept>

struct Exception: public std::runtime_error
{
    Exception(std::string const& message)
            : std::runtime_error(message)
    {}
};
