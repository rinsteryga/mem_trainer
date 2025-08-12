#include "../include/MainLoop.hpp"
#include "../include/Menu.hpp"

#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <iterator>
#include <cstdlib>
#include <utility>

MainLoop::MainLoop()
    : db_sync(),
      current_user_id(-1)
{
    try
    {
        auto menu = std::make_unique<Menu>();
        uint16_t attempts{0};
        bool connected{false};
        while (attempts < 3 && !connected)
        {
            if (db_sync.connect())
            {
                connected = true;
            }
            else
            {
                attempts++;
                if (attempts < 3)
                {
                    menu->print_message("Retrying connection... (" + std::to_string(attempts) + "/3)\n");
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
        }
        if (!connected)
        {
            throw std::runtime_error("Failed to connect to database after 3 attempts");
        }
        menu->print_message("Connected to database successfully.\n");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "Please check your config.ini file\n";
        exit(1);
    }
}

MainLoop::~MainLoop()
{
    if (db_sync.get_connection())
    {
        PQfinish(db_sync.get_connection());
    }
}

bool MainLoop::authenticate_user()
{
    auto menu = std::make_unique<Menu>();

    std::string username, password;

    menu->print_message("\nEnter username: ");
    std::getline(std::cin, username);

    menu->print_message("Enter password: ");
    std::getline(std::cin, password);

    const char *paramValues[2] = {username.c_str(), password.c_str()};
    int32_t paramLengths[2] = {static_cast<int32_t>(username.length()), static_cast<int32_t>(password.length())};
    int32_t paramFormats[2] = {0, 0}; // 0 = text

    PGresult *res = PQexecParams(db_sync.get_connection(),
                                 "SELECT id FROM users WHERE username = $1 AND password = $2",
                                 2, NULL, paramValues, paramLengths, paramFormats, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Authentication failed: " << PQerrorMessage(db_sync.get_connection()) << "\n";
        PQclear(res);
        return false;
    }

    bool success = (PQntuples(res) == 1);
    if (success)
    {
        current_user_id = std::stoi(PQgetvalue(res, 0, 0));
        menu->print_message("Login successful!\n");
    }
    else
    {
        menu->print_message("Invalid username or password.\n");
    }

    PQclear(res);
    return success;
}

bool MainLoop::register_user() const
{
    auto menu = std::make_unique<Menu>();

    std::string username, password;

    menu->print_message("\nEnter new username: ");
    std::getline(std::cin, username);

    menu->print_message("Enter new password: ");
    std::getline(std::cin, password);

    const char *paramValues[2] = {username.c_str(), password.c_str()};
    int32_t paramLengths[2] = {static_cast<int32_t>(username.length()), static_cast<int32_t>(password.length())};
    int32_t paramFormats[2] = {0, 0}; // 0 = text

    PGresult *res = PQexecParams(db_sync.get_connection(),
                                 "INSERT INTO users (username, password) VALUES ($1, $2)",
                                 2, NULL, paramValues, paramLengths, paramFormats, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        std::cerr << "Registration failed: " << PQerrorMessage(db_sync.get_connection()) << "\n";
        PQclear(res);
        return false;
    }

    PQclear(res);
    menu->print_message("Registration successful! You can now login.\n");
    return true;
}

void MainLoop::start_training()
{
    auto menu = std::make_unique<Menu>();

    int32_t difficulty_level{db_sync.get_user_difficulty(current_user_id)};
    TaskGenerator::Difficulty difficulty = static_cast<TaskGenerator::Difficulty>(difficulty_level);

    TaskGenerator generator(difficulty);
    auto sequence = generator.generate_sequence(
        generator.get_params_for_difficulty(difficulty).min_length);

    display_training_header(difficulty, sequence.size());
    display_sequence(sequence);

    uint32_t memorization_time;
    switch (difficulty)
    {
    case TaskGenerator::Difficulty::EASY:
        memorization_time = 7;
        break; // EASY
    case TaskGenerator::Difficulty::MEDIUM:
        memorization_time = 6;
        break; // MEDIUM
    case TaskGenerator::Difficulty::HARD:
        memorization_time = 5;
        break; // HARD
    }
    std::ostringstream oss;
    oss << "\n\nYou have " << memorization_time << " seconds to remember...\n";
    menu->print_message(oss.str());
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(memorization_time);

    while (std::chrono::steady_clock::now() < end_time)
    {
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
                             end_time - std::chrono::steady_clock::now())
                             .count();

        std::ostringstream oss;
        oss << "\rTime left: " << remaining << " seconds";
        menu->print_message(oss.str());
        std::cout << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;

    clear_screen();
    auto user_answers = prompt_user_input();

    if (user_answers.size() != sequence.size())
    {
        std::cerr << "Please enter exactly " << sequence.size() << " items." << std::endl;
    }

    uint32_t correct = check_answers(sequence, user_answers);
    float success_rate = static_cast<float>(correct) / sequence.size();
    uint32_t score = calculate_score(success_rate, difficulty);

    save_training_results(sequence.size(), success_rate, score);
    update_difficulty_if_needed(difficulty, success_rate);

    print_results(correct, sequence.size(), success_rate, score, difficulty);
}

void MainLoop::display_training_header(TaskGenerator::Difficulty difficulty, size_t sequence_length)
{
    auto menu = std::make_unique<Menu>();
    menu->print_message("\n=== Memory Training ===\n");
    menu->print_message("Difficulty: ");
    switch (difficulty)
    {
    case TaskGenerator::Difficulty::EASY:
        menu->print_message("EASY");
        break;
    case TaskGenerator::Difficulty::MEDIUM:
        menu->print_message("MEDIUM");
        break;
    case TaskGenerator::Difficulty::HARD:
        menu->print_message("HARD");
        break;
    }
    menu->print_message("\nRemember this sequence (" +
                        std::to_string(sequence_length) +
                        " items):\n");
}

void MainLoop::display_sequence(const std::vector<TaskGenerator::TaskItem> &sequence)
{
    auto menu = std::make_unique<Menu>();
    for (const auto &item : sequence)
    {
        std::visit([&menu](auto &&arg)
                   {
            std::ostringstream oss;
            oss << arg << " ";
            menu->print_message(oss.str()); }, item);
    }
}

void MainLoop::clear_screen()
{
#ifdef _WIN32
    system("cls"); // Windows
#else
    system("clear"); // Linux/macOS
#endif
}

std::vector<std::string> MainLoop::prompt_user_input()
{
    auto menu = std::make_unique<Menu>();

    menu->print_message("Enter the sequence (separate items with spaces):\n");
    std::string input;
    std::getline(std::cin, input);

    std::istringstream iss(input);
    return {std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>{}};
}

uint32_t MainLoop::check_answers(const std::vector<TaskGenerator::TaskItem> &sequence,
                                 const std::vector<std::string> &user_answers) const
{
    uint32_t correct = 0;
    for (size_t i{0}; i < sequence.size(); ++i)
    {
        if (i < user_answers.size() && !user_answers[i].empty())
        {
            bool is_correct = false;
            std::visit([&](auto &&arg)
                       {
                using T = std::decay_t<decltype(arg)>;
                try {
                    if constexpr (std::is_same_v<T, std::string>) {
                        // сравнение строк (для слов)
                        is_correct = (arg == user_answers[i]);
                    } 
                    else if constexpr (std::is_same_v<T, char>) {
                        // сравнение символов (регистронезависимо)
                        is_correct = (std::tolower(arg) == std::tolower(user_answers[i][0]));
                    } 
                    else if constexpr (std::is_integral_v<T>) {
                        // сравнение целых чисел
                        is_correct = (arg == std::stoi(user_answers[i]));
                    } 
                    else if constexpr (std::is_floating_point_v<T>) {
                        // сравнение float с погрешностью
                        const float user_value = std::stof(user_answers[i]);
                        if (user_value == arg) 
                        {
                        is_correct = true;
                        }
                        else 
                        {
                            const float epsilon = 0.01f;
                            is_correct = (std::abs(arg - user_value) < epsilon);
                        }
                    }
                } 
                catch (...) {
                    is_correct = false;
                } }, sequence[i]);

            if (is_correct)
                correct++;
        }
    }
    return correct;
}

uint32_t MainLoop::calculate_score(float success_rate, TaskGenerator::Difficulty difficulty) const
{
    return static_cast<uint32_t>(success_rate * 100 * (static_cast<uint32_t>(difficulty) + 1));
}

void MainLoop::save_training_results(size_t sequence_length, float success_rate, uint32_t score)
{
    const char *progress_params[3] = {
        std::to_string(current_user_id).c_str(),
        std::to_string(sequence_length).c_str(),
        std::to_string(success_rate).c_str()};
    PGresult *progress_res = PQexecParams(db_sync.get_connection(),
                                          "INSERT INTO user_progress (user_id, sequence_length, success_rate) VALUES ($1, $2, $3)",
                                          3, NULL, progress_params, NULL, NULL, 0);
    PQclear(progress_res);

    db_sync.update_score(current_user_id, score);
}

void MainLoop::update_difficulty_if_needed(TaskGenerator::Difficulty difficulty, float success_rate)
{
    if (success_rate > 0.75f && difficulty != TaskGenerator::Difficulty::HARD)
    {
        uint32_t new_level = static_cast<uint32_t>(difficulty) + 1;
        db_sync.update_difficulty(current_user_id, new_level);
    }
    else if (success_rate < 0.3f && difficulty != TaskGenerator::Difficulty::EASY)
    {
        uint32_t new_level = static_cast<uint32_t>(difficulty) - 1;
        db_sync.update_difficulty(current_user_id, new_level);
    }
}

void MainLoop::print_results(uint32_t correct, std::size_t total, float success_rate,
                             uint32_t score, TaskGenerator::Difficulty difficulty) const
{
    auto menu = std::make_unique<Menu>();
    bool level_increased = (success_rate > 0.75f &&
                            difficulty != TaskGenerator::Difficulty::HARD);
    bool suggest_easier = (success_rate < 0.3f &&
                           difficulty != TaskGenerator::Difficulty::EASY);

    menu->print_training_results(correct, total, success_rate, score,
                                 level_increased, suggest_easier);
}

void MainLoop::show_leaderboard() const
{
    if (!db_sync.get_connection() || PQstatus(db_sync.get_connection()) != CONNECTION_OK)
    {
        std::cerr << "Failure: no connection to db.\n";
        return;
    }

    // топ-10 игроков по очкам
    const char *query =
        "SELECT username, total_score FROM users "
        "WHERE total_score > 0 "
        "ORDER BY total_score DESC "
        "LIMIT 10";

    PGresult *res = PQexec(db_sync.get_connection(), query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Failure on getting leaderboard: " << PQerrorMessage(db_sync.get_connection()) << "\n";
        PQclear(res);
        return;
    }

    // вывод leaderboard
    auto menu = std::make_unique<Menu>();
    uint32_t rows = PQntuples(res);
    std::vector<std::pair<std::string, std::string>> leaders;

    if (rows == 0)
    {
        menu->print_message("\nLeaderboard is clear. Be first!\n");
    }
    else
    {
        for (std::size_t i = 0; i < rows; ++i)
        {
            leaders.emplace_back(
                PQgetvalue(res, i, 0), // username
                PQgetvalue(res, i, 1)  // score
            );
        }
        menu->print_leaderboard(leaders);
    }

    PQclear(res);
}

void MainLoop::show_user_progress()
{
    auto menu = std::make_unique<Menu>();
    auto progress = db_sync.get_user_progress(current_user_id);

    menu->print_message("\n=== Your Training History ===\n");
    for (const auto &record : progress)
    {
        std::ostringstream oss;
        oss << "Date: " << record.training_date.substr(0, 10)
            << " | Length: " << record.sequence_length
            << " items | Success: " << std::fixed << std::setprecision(1)
            << (record.success_rate * 100) << "%";
        menu->print_message(oss.str());
    }
}

void MainLoop::run()
{
    auto menu = std::make_unique<Menu>();
    bool authenticated = false;

    while (!authenticated)
    {
        menu->print_auth_menu();

        uint32_t choice;
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            menu->print_message("Please enter a number.\n");
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            if (authenticate_user())
            {
                authenticated = true;
            }
            break;
        case 2:
            register_user();
            break;
        case 3:
            menu->print_message("Exiting...\n");
            return;
        default:
            menu->print_message("Invalid choice. Try again.\n");
        }
    }

    while (true)
    {
        menu->print_main_menu();

        uint32_t choice;
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            menu->print_message("Please enter a number\n");
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            start_training();
            break;
        case 2:
            show_leaderboard();
            break;
        case 3:
            return;
        default:
            menu->print_message("Invalid choice. Try again.\n");
        }
    }
}