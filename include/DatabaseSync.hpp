#pragma once

#include <memory>
#include <libpq-fe.h>
#include <string>
#include <vector>
#include <cstdint>

struct UserProgress
{
    int32_t sequence_length;
    float success_rate;
    std::string training_date;
};

class DatabaseSync
{
public:
    DatabaseSync(const std::string &conninfo);
    DatabaseSync();
    ~DatabaseSync();

    std::string parse_config_file();
    bool connect();
    PGconn* get_connection() const;
    bool save_progress(uint32_t user_id, uint32_t sequence_length, float success_rate);
    bool update_difficulty(uint32_t user_id, uint32_t new_level);
    bool update_score(uint32_t user_id, uint32_t score_delta);
    std::vector<UserProgress> get_user_progress(uint32_t user_id);
    int32_t get_user_difficulty(uint32_t user_id) const;

private:
    std::shared_ptr<PGconn> db_connection;
    std::string connection_info;

    PGresult *execute_params(const char *query,
                             const std::vector<const char *> &values,
                             const std::vector<int32_t> &lengths);
};
