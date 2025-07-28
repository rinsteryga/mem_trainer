#pragma once

#include <cstdint>

template <typename T>
class Value
{
public:
    void generate_value(uint16_t difficulty_id);
    T get() const noexcept;

private:
    T m_RandValue;
};