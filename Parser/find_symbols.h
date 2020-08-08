#pragma once

#include <cstdint>

template <char s0>
inline bool is_in(char x)
{
    return x == s0;
}

template <char s0, char s1, char... tail>
inline bool is_in(char x)
{
    return x == s0 || is_in<s1, tail...>(x);
}

template <bool positive>
uint16_t maybe_negate(uint16_t x)
{
    if constexpr (positive)
        return x;
    else
        return ~x;
}

enum class ReturnMode
{
    End,
    Nullptr,
};

template <bool positive, ReturnMode return_mode, char... symbols>
inline const char * find_first_symbols_sse2(const char * const begin, const char * const end)
{
    const char * pos = begin;

    for (; pos < end; ++pos)
        if (maybe_negate<positive>(is_in<symbols...>(*pos)))
            return pos;

    return return_mode == ReturnMode::End ? end : nullptr;
}

template <bool positive, ReturnMode return_mode, char... symbols>
inline const char * find_first_symbols_dispatch(const char * begin, const char * end)
{
    return find_first_symbols_sse2<positive, return_mode, symbols...>(begin, end);
}

template <char... symbols>
inline const char * find_first_symbols(const char * begin, const char * end)
{
    return find_first_symbols_dispatch<true, ReturnMode::End, symbols...>(begin, end);
}