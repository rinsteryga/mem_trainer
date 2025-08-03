#include "..\include\TaskGenerator.hpp"
#include "..\include\RandomGenerators.hpp"

#include <random>
#include <algorithm>

namespace
{
    // генератор случайных чисел
    auto &get_generator()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }

    // выбор случайного типа числа
    TaskGenerator::TaskItem generate_random_number()
    {
        static std::uniform_int_distribution<int> dist(0, 2);
        switch (dist(get_generator()))
        {
        case 0:
            return NumberGenerator::generate_uint16();
        case 1:
            return NumberGenerator::generate_uint32();
        case 2:
            return NumberGenerator::generate_float();
        }
    }
}

TaskGenerator::TaskGenerator(Difficulty initial_difficulty)
    : current_difficulty(initial_difficulty) {}

void TaskGenerator::set_difficulty(Difficulty new_difficulty)
{
    current_difficulty = new_difficulty;
}

std::vector<TaskGenerator::TaskItem> TaskGenerator::generate_sequence(size_t length)
{
    const auto params = get_params_for_difficulty(current_difficulty);
    length = std::clamp(length, params.min_length, params.max_length);

    auto& gen = get_generator();
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    if (params.mixed_types && dist(gen) > 0.5f) {
        std::vector<TaskItem> result;
        result.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            const float choice = dist(gen);
            if (choice < 0.4f) {
                result.push_back(::generate_random_number());  // Используем вашу функцию
            }
            else if (choice < 0.7f) {
                result.push_back(TaskItem{SymbolGenerator::generate_char()});
            }
            else {
                result.push_back(TaskItem{WordGenerator::generate_word()});
            }
        }
        return result;
    }

    if (dist(gen) < params.float_probability) {
        return generate_number_sequence(length);
    }
    return (dist(gen) < 0.6f) ? generate_symbol_sequence(length)
                              : generate_word_sequence(length);
}

std::vector<TaskGenerator::TaskItem> TaskGenerator::generate_number_sequence(size_t length) const // принимает количество чисел для генерации
{
    std::vector<TaskItem> result;
    result.reserve(length);
    std::generate_n(std::back_inserter(result), length, generate_random_number);
    return result;
}

std::vector<TaskGenerator::TaskItem> TaskGenerator::generate_symbol_sequence(size_t length) const // принимает количество символов для генерации
{
    std::vector<TaskItem> result;
    result.reserve(length);
    std::generate_n(std::back_inserter(result), length,
                    []
                    { return SymbolGenerator::generate_char(); });
    return result;
}

std::vector<TaskGenerator::TaskItem> TaskGenerator::generate_word_sequence(size_t length) const // принимает количество слов для генерации
{
    std::vector<TaskItem> result;
    result.reserve(length);
    std::generate_n(std::back_inserter(result), length,
                    []
                    { return WordGenerator::generate_word(); });
    return result;
}

TaskGenerator::DifficultyParams TaskGenerator::get_params_for_difficulty(
    Difficulty level) noexcept
{
    static constexpr DifficultyParams params[] = {
        // EASY
        {3, 5, 0.3f, false},
        // MEDIUM
        {5, 8, 0.5f, true},
        // HARD
        {8, 12, 0.7f, true}};
    return params[static_cast<int>(level)];
}
