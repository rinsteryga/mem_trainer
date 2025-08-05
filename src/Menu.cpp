#include "..\include\Menu.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace
{
    constexpr const char *GRAY = "\033[38;2;180;180;180m";
    constexpr const char *ITALIC = "\033[3m";
    constexpr const char *BOLD = "\033[1m";
    constexpr const char *RESET = "\033[0m";

    void print_menu(const char *title, const char *options, const char *frame_char, int frame_length)
    {
        std::cout << GRAY << ITALIC << "\n";

        // верхняя рамка с заголовком
        std::cout << std::string(frame_length / 2 - 2, *frame_char) << " "
                  << BOLD << title << RESET << GRAY << ITALIC << " "
                  << std::string(frame_length / 2 - 2, *frame_char) << "\n";

        // опции меню
        std::cout << options;

        // нижняя рамка
        std::cout << std::string(frame_length, *frame_char) << "\n"
                  << "Select an option: " << RESET;
    }
}

void Menu::print_auth_menu() const
{
    print_menu(
        "Memory Trainer",
        "1. Login\n"
        "2. Register\n"
        "3. Exit\n",
        "=", 30);
}

void Menu::print_main_menu() const
{
    print_menu(
        "Main Menu",
        "1. Start Training\n"
        "2. View Leaderboard\n"
        "3. Logout and exit\n",
        "=", 24);
}

void Menu::print_leaderboard(const std::vector<std::pair<std::string, std::string>> &leaders) const
{
    constexpr const char *GRAY = "\033[38;2;180;180;180m";
    constexpr const char *ITALIC = "\033[3m";
    constexpr const char *BOLD = "\033[1m";
    constexpr const char *RESET = "\033[0m";

    std::cout << GRAY << ITALIC
              << "\n======= " << BOLD << "TOP-10 Players" << RESET << GRAY << ITALIC << " =======\n"
              << RESET;

    std::cout << GRAY << BOLD
              << std::left << std::setw(4) << "#"
              << std::setw(20) << "Name"
              << "Score" << RESET << "\n";

    for (std::size_t i = 0; i < leaders.size(); ++i)
    {
        std::cout << GRAY << ITALIC
                  << std::setw(4) << i + 1
                  << std::setw(20) << leaders[i].first   // username
                  << leaders[i].second << RESET << "\n"; // score
    }

    std::cout << GRAY << ITALIC
              << "=============================="
              << RESET << "\n";
}

void Menu::print_message(const std::string &message) const
{
    constexpr const char *GRAY = "\033[38;2;180;180;180m";
    constexpr const char *RESET = "\033[0m";

    std::cout << GRAY << message << RESET;
}

void Menu::print_training_results(uint32_t correct, size_t total, float success_rate,
                                  uint32_t score, bool level_increased, bool suggest_easier) const
{
    std::ostringstream oss;

    oss << "\nTraining results:\n"
        << "Correct: " << correct << "/" << total << "\n"
        << "Success rate: " << std::fixed << std::setprecision(1)
        << (success_rate * 100) << "%\n"
        << "Points earned: " << score << "\n";

    if (level_increased)
    {
        oss << "Congratulations! Difficulty level increased!\n";
    }
    else if (suggest_easier)
    {
        oss << "Try easier difficulty next time!\n";
    }

    print_message(oss.str());
}
