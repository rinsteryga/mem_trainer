#pragma once

#include "../include/DatabaseSync.hpp"
#include "../include/TaskGenerator.hpp"

#include <memory>
#include <libpq-fe.h>
#include <cstdint>

class MainLoop
{
public:
    MainLoop();
    ~MainLoop();

    void run();

private:
    bool authenticate_user();
    bool register_user() const;
    void start_training();
    void show_leaderboard() const;
    void display_training_header(TaskGenerator::Difficulty difficulty, std::size_t sequence_length);
    void display_sequence(const std::vector<TaskGenerator::TaskItem> &sequence);
    void clear_screen();
    std::vector<std::string> prompt_user_input();
    uint32_t check_answers(const std::vector<TaskGenerator::TaskItem> &sequence,
                           const std::vector<std::string> &user_answers) const;
    void save_training_results(std::size_t sequence_length, float success_rate, uint32_t score);
    void update_difficulty_if_needed(TaskGenerator::Difficulty difficulty, float success_rate);
    void print_results(uint32_t correct, std::size_t total, float success_rate,
                       uint32_t score, TaskGenerator::Difficulty difficulty) const;
    void show_user_progress();
    uint32_t calculate_score(float, TaskGenerator::Difficulty difficulty) const;
    DatabaseSync db_sync;
    std::shared_ptr<PGconn> db_connection;
    int32_t current_user_id;
};
