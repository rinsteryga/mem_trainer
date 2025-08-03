#include "..\include\MainLoop.hpp"
#include "..\include\Menu.hpp"

#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <iterator>

MainLoop::MainLoop()
    : db_sync("dbname=mem_trainer user=postgres password=ruslan host=localhost port=5432"),
      current_user_id(-1)
{
    db_connection = db_sync.get_connection();

    if (!db_connection || PQstatus(db_connection.get()) != CONNECTION_OK)
    {
        std::cerr << "Failed to connect to database. Exiting...\n";
        exit(1);
    }
}
MainLoop::~MainLoop()
{
    if (db_connection)
    {
        PQfinish(db_connection.get());
    }
}

bool MainLoop::connect_to_database()
{
    const char *conninfo = "dbname=mem_trainer user=postgres password=ruslan host=localhost port=5432";
    db_connection = std::shared_ptr<PGconn>(PQconnectdb(conninfo), PQfinish);

    if (PQstatus(db_connection.get()) != CONNECTION_OK)
    {
        std::cerr << "Connection to database failed: " << PQerrorMessage(db_connection.get());
        return false;
    }

    std::cout << "Connected to database successfully.\n";
    return true;
}

bool MainLoop::authenticate_user() const
{
    std::string username, password;

    std::cout << "\nEnter username: ";
    std::getline(std::cin, username);

    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    const char *paramValues[2] = {username.c_str(), password.c_str()};
    int32_t paramLengths[2] = {static_cast<int32_t>(username.length()), static_cast<int32_t>(password.length())};
    int32_t paramFormats[2] = {0, 0}; // 0 = text

    PGresult *res = PQexecParams(db_connection.get(),
                                 "SELECT id FROM users WHERE username = $1 AND password = $2",
                                 2, NULL, paramValues, paramLengths, paramFormats, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Authentication failed: " << PQerrorMessage(db_connection.get()) << "\n";
        PQclear(res);
        return false;
    }

    bool success = (PQntuples(res) == 1);
    PQclear(res);

    if (success)
    {
        std::cout << "Login successful!\n";
    }
    else
    {
        std::cout << "Invalid username or password.\n";
    }

    return success;
}

bool MainLoop::register_user() const
{
    std::string username, password;

    std::cout << "\nEnter new username: ";
    std::getline(std::cin, username);

    std::cout << "Enter new password: ";
    std::getline(std::cin, password);

    const char *paramValues[2] = {username.c_str(), password.c_str()};
    int32_t paramLengths[2] = {static_cast<int32_t>(username.length()), static_cast<int32_t>(password.length())};
    int32_t paramFormats[2] = {0, 0}; // 0 = text

    PGresult *res = PQexecParams(db_connection.get(),
                                 "INSERT INTO users (username, password) VALUES ($1, $2)",
                                 2, NULL, paramValues, paramLengths, paramFormats, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        std::cerr << "Registration failed: " << PQerrorMessage(db_connection.get()) << "\n";
        PQclear(res);
        return false;
    }

    PQclear(res);
    std::cout << "Registration successful! You can now login.\n";
    return true;
}

void MainLoop::start_training()
{
    int32_t difficulty_level{db_sync.get_user_difficulty(current_user_id)};
    TaskGenerator::Difficulty difficulty = static_cast<TaskGenerator::Difficulty>(difficulty_level);

    TaskGenerator generator(difficulty);
    auto sequence = generator.generate_sequence(
        generator.get_params_for_difficulty(difficulty).min_length);

    display_training_header(difficulty, sequence.size());
    display_sequence(sequence);

    uint32_t memorization_time = 5 - static_cast<uint32_t>(difficulty);
    std::cout << "\n\nYou have " << memorization_time << " seconds to remember...\n";
    std::this_thread::sleep_for(std::chrono::seconds(memorization_time));

    clear_screen();
    auto user_answers = prompt_user_input();

    uint32_t correct = check_answers(sequence, user_answers);
    float success_rate = static_cast<float>(correct) / sequence.size();
    uint32_t score = calculate_score(success_rate, difficulty);

    save_training_results(sequence.size(), success_rate, score);
    update_difficulty_if_needed(difficulty, success_rate);

    print_results(correct, sequence.size(), success_rate, score, difficulty);
}

void MainLoop::display_training_header(TaskGenerator::Difficulty difficulty, size_t sequence_length)
{
    std::cout << "\n=== Memory Training ===\n";
    std::cout << "Difficulty: ";
    switch (difficulty)
    {
    case TaskGenerator::Difficulty::EASY:
        std::cout << "EASY";
        break;
    case TaskGenerator::Difficulty::MEDIUM:
        std::cout << "MEDIUM";
        break;
    case TaskGenerator::Difficulty::HARD:
        std::cout << "HARD";
        break;
    }
    std::cout << "\nRemember this sequence (" << sequence_length << " items):\n";
}

void MainLoop::display_sequence(const std::vector<TaskGenerator::TaskItem> &sequence)
{
    for (const auto &item : sequence)
    {
        std::visit([](auto &&arg)
                   { std::cout << arg << " "; }, item);
    }
}

void MainLoop::clear_screen()
{
    for (unsigned short i{0}; i < 30; ++i)
    {
        std::cout << "\n";
    }
}

std::vector<std::string> MainLoop::prompt_user_input()
{
    std::cout << "Enter the sequence (separate items with spaces):\n";
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
    for (size_t i = 0; i < std::min(sequence.size(), user_answers.size()); ++i)
    {
        bool is_correct = false;
        std::visit([&](auto &&arg)
                   {
            using T = std::decay_t<decltype(arg)>;
            try {
                if constexpr (std::is_same_v<T, std::string>) {
                    is_correct = (arg == user_answers[i]);
                } else if constexpr (std::is_integral_v<T>) {
                    is_correct = (arg == std::stoi(user_answers[i]));
                } else if constexpr (std::is_floating_point_v<T>) {
                    is_correct = (std::abs(arg - std::stof(user_answers[i])) < 0.001f);
                }
            } catch (...) {
                is_correct = false;
            } }, sequence[i]);

        if (is_correct)
            correct++;
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
    PGresult *progress_res = PQexecParams(db_connection.get(),
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
}

void MainLoop::print_results(uint32_t correct, size_t total, float success_rate,
                             uint32_t score, TaskGenerator::Difficulty difficulty) const
{
    std::cout << "\nTraining results:\n";
    std::cout << "Correct: " << correct << "/" << total << "\n";
    std::cout << "Success rate: " << std::fixed << std::setprecision(1) << (success_rate * 100) << "%\n";
    std::cout << "Points earned: " << score << "\n";

    if (success_rate > 0.75f && difficulty != TaskGenerator::Difficulty::HARD)
    {
        std::cout << "Congratulations! Difficulty level increased!\n";
    }
    else if (success_rate < 0.3f && difficulty != TaskGenerator::Difficulty::EASY)
    {
        std::cout << "Try easier difficulty next time!\n";
    }
}

void MainLoop::show_leaderboard() const
{
    PGresult *res = PQexec(db_connection.get(),
                           "SELECT username, score FROM users ORDER BY score DESC LIMIT 10");

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cerr << "Error getting leaderboard: " << PQerrorMessage(db_connection.get()) << "\n";
        PQclear(res);
        return;
    }

    std::cout << "\n======= Leaderboard =======\n";
    uint32_t rows = PQntuples(res);
    for (short i{0}; i < rows; ++i)
    {
        std::cout << (i + 1) << ". " << PQgetvalue(res, i, 0)
                  << " - Score: " << PQgetvalue(res, i, 1) << "\n";
    }
    std::cout << "===========================\n";

    PQclear(res);
}

void MainLoop::show_user_progress()
{
    auto progress = db_sync.get_user_progress(current_user_id);

    std::cout << "\n=== Your Training History ===\n";
    for (const auto &record : progress)
    {
        std::cout << "Date: " << record.training_date.substr(0, 10) // Обрезаем время
                  << " | Length: " << record.sequence_length
                  << " items | Success: " << std::fixed << std::setprecision(1)
                  << (record.success_rate * 100) << "%\n";
    }
}

void MainLoop::run()
{
    auto menu = std::make_unique<Menu>();
    while (true)
    {
        menu->print_main_menu();

        uint32_t choice;
        std::cin >> choice;
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
            show_user_progress();
            break;
        case 4:
            return;
        default:
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}
