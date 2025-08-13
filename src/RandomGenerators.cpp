#include "../include/RandomGenerators.hpp"

#include <random>
#include <algorithm>
#include <iterator>

namespace
{
    inline auto &get_generator() noexcept
    {
        thread_local static std::random_device rd;
        thread_local static std::mt19937 gen(rd());
        return gen;
    }

    template <typename T>
    T generate_integer(T min = std::numeric_limits<T>::min(),
                       T max = std::numeric_limits<T>::max())
    {
        if constexpr (std::is_integral_v<T>)
        {
            max = std::min(max, static_cast<T>(99999)); // Не больше 5 цифр
        }
        std::uniform_int_distribution<T> dist(min, max);
        return dist(get_generator());
    }

    template <>
    float generate_integer<float>(float min, float max)
    {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(get_generator());
    }
}

uint16_t NumberGenerator::generate_uint16() noexcept
{
    return generate_integer<uint16_t>();
}

uint32_t NumberGenerator::generate_uint32() noexcept
{
    return generate_integer<uint32_t>();
}

float NumberGenerator::generate_float() noexcept
{
    return std::round(generate_integer<float>(0.0f, 10.0f) * 1000) / 1000;
}

char SymbolGenerator::generate_char() noexcept
{
    constexpr auto size = std::size(symbols);
    return symbols[generate_integer<std::size_t>(0, size - 1)];
}

std::string SymbolGenerator::generate_string(std::size_t length) noexcept
{
    std::string result;
    result.reserve(length);
    std::generate_n(std::back_inserter(result), length,
                    []()
                    { return generate_char(); });
    return result;
}

std::string WordGenerator::generate_word() noexcept
{
    constexpr auto size = std::size(words);
    return words[generate_integer<std::size_t>(0, size - 1)];
}
