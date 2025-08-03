#include "..\include\Menu.hpp"

#include <iostream>

void Menu::print_auth_menu() const
{
    static constexpr const char *header = "\n======= Memory Trainer =======\n";
    static constexpr const char *options =
        "1. Login\n"
        "2. Register\n"
        "3. View Progress\n"
        "4. Exit\n";
    static constexpr const char *footer = "=============================\n"
                                          "Select an option: ";

    std::cout << header << options << footer;
}

void Menu::print_main_menu() const
{
    static constexpr const char *header = "\n======= Main Menu =======\n";
    static constexpr const char *options =
        "1. Start Training\n"
        "2. View Leaderboard\n"
        "3. Logout\n";
    static constexpr const char *footer = "========================\n"
                                          "Select an option: ";

    std::cout << header << options << footer;
}
