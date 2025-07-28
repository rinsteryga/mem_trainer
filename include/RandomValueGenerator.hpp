#pragma once

#include <cstdint>

class RandomValueGenerator
{
public:
    static uint16_t generate_uint16() noexcept;
    static uint32_t generate_uint32() noexcept;
    static float generate_float() noexcept;
};