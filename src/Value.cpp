#include "..\include\Value.hpp"

#include "RandomValueGenerator.hpp"

template <typename T>
void Value<T>::generate_value(uint16_t difficulty_id)
{
    switch (difficulty_id)
    {
    case 1: // для uint16
        m_RandValue = static_cast<T>(RandomValueGenerator::generate_uint16());
        break;
    case 2: // для uint32
        m_RandValue = static_cast<T>(RandomValueGenerator::generate_uint32());
        break;
    case 3: // для float
        m_RandValue = static_cast<T>(RandomValueGenerator::generate_float());
        break;
    default:
        m_RandValue = T{};
        break;
    }
}

template <typename T>
T Value<T>::get() const noexcept
{
    return m_RandValue;
}

template class Value<uint16_t>;
template class Value<uint32_t>;
template class Value<float>;