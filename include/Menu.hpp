#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdint>
#include <cstddef>

class Menu
{
public:
    void print_auth_menu() const;
    void print_main_menu() const;
    void print_leaderboard(const std::vector<std::pair<std::string, std::string>> &leaders) const;
    void print_message(const std::string &message) const;
    void print_training_results(uint32_t correct, std::size_t total, float success_rate,
                                uint32_t score, bool level_increased, bool suggest_easier) const;
};
