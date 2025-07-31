#pragma once

#include <vector>
#include <string>
#include <variant>
#include <cstddef>

class TaskGenerator
{
public:
    enum class Difficulty
    {
        EASY,
        MEDIUM,
        HARD
    };
    using TaskItem = std::variant<uint16_t, uint32_t, float, char, std::string>;

    TaskGenerator(Difficulty initial_difficulty = Difficulty::MEDIUM);

    void set_difficulty(Difficulty new_difficulty);
    Difficulty get_difficulty() const noexcept;

    std::vector<TaskItem> generate_sequence(std::size_t length);

private:
    Difficulty current_difficulty;

    std::vector<TaskItem> generate_number_sequence(std::size_t length) const;
    std::vector<TaskItem> generate_symbol_sequence(std::size_t length) const;
    std::vector<TaskItem> generate_word_sequence(std::size_t length) const;

    struct DifficultyParams
    {
        std::size_t min_length;
        std::size_t max_length;
        float float_probability;
        bool mixed_types;
    };

    static DifficultyParams get_params_for_difficulty(Difficulty level) noexcept;
};
