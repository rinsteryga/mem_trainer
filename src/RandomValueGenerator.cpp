#include "..\include\RandomValueGenerator.hpp"

#include <random>

namespace
{
    template <typename T>
    T generate_random()
    {
        static std::random_device rd;
        static std::mt19937 generator(rd());

        if constexpr (std::is_integral_v<T>)
        {
            static std::uniform_int_distribution<T> distribution;
            return distribution(generator);
        }
        else
        {
            static std::uniform_real_distribution<T> distribution(0.0f, 1.0f);
            return distribution(generator);
        }
    }
}

uint16_t RandomValueGenerator::generate_uint16() noexcept
{
    return generate_random<uint16_t>();
}

uint32_t RandomValueGenerator::generate_uint32() noexcept
{
    return generate_random<uint32_t>();
}

float RandomValueGenerator::generate_float() noexcept
{
    return generate_random<float>();
}