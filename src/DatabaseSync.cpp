#include "..\include\DatabaseSync.hpp"

#include <iostream>
#include <fstream>
#include <system_error>
#include <format>
#include <unordered_map>

DatabaseSync::DatabaseSync(const std::string &conninfo)
    : connection_info(conninfo)
{
    std::cout << "Using explicit connection parameters\n";
}

// config.ini
DatabaseSync::DatabaseSync()
{
    connection_info = parse_config_file();
    if (connection_info.empty())
    {
        throw std::runtime_error("Invalid database configuration in config.ini");
    }
    std::cout << "Using config.ini file connection\n";
}

DatabaseSync::~DatabaseSync()
{
    if (db_connection)
    {
        PQfinish(db_connection.get());
    }
}

std::string DatabaseSync::parse_config_file()
{
    std::ifstream config("config.ini");
    if (!config.is_open())
    {
        throw std::runtime_error("Configuration file config.ini not found");
    }

    std::unordered_map<std::string, std::string> params = {
        {"host", "localhost"},
        {"port", "5432"},
        {"dbname", ""},
        {"user", ""},
        {"password", ""}};

    std::string line;
    while (getline(config, line))
    {
        size_t comment_pos = line.find_first_of('#');
        if (comment_pos != std::string::npos)
        {
            line.erase(comment_pos);
        }

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        for (auto &[key, _] : params)
        {
            size_t pos = line.find(key + "=");
            if (pos != std::string::npos && pos == 0)
            {
                std::string value = line.substr(key.length() + 1);

                if (!value.empty())
                {
                    if (value.front() == '"' || value.front() == '\'')
                    {
                        value.erase(0, 1);
                    }
                    if (value.back() == '"' || value.back() == '\'')
                    {
                        value.pop_back();
                    }
                }

                params[key] = value;
                break;
            }
        }
    }

    if (params["dbname"].empty() || params["user"].empty() || params["password"].empty())
    {
        throw std::runtime_error("Missing required database parameters in config.ini");
    }

    return std::format(
        "host={} port={} dbname={} user={} password={}",
        params["host"], params["port"], params["dbname"],
        params["user"], params["password"]);
}

bool DatabaseSync::connect()
{
    db_connection = std::shared_ptr<PGconn>(
        PQconnectdb(connection_info.c_str()),
        PQfinish);

    if (PQstatus(db_connection.get()) != CONNECTION_OK)
    {
        std::cerr << "PQerrorMessage: " << PQerrorMessage(db_connection.get()) << std::endl;
        return false;
    }

    return true;
}

PGresult *DatabaseSync::execute_params(const char *query,
                                       const std::vector<const char *> &values,
                                       const std::vector<int32_t> &lengths)
{
    return PQexecParams(
        get_connection(),
        query,
        values.size(),
        nullptr,
        values.data(),
        lengths.data(),
        nullptr,
        0);
}

bool DatabaseSync::save_progress(uint32_t user_id, uint32_t sequence_length, float success_rate)
{
    std::string sl_str = std::to_string(sequence_length);
    std::string sr_str = std::to_string(success_rate);
    std::string uid_str = std::to_string(user_id);

    std::vector<const char *> values = {
        uid_str.c_str(),
        sl_str.c_str(),
        sr_str.c_str()};

    std::vector<int32_t> lengths(values.size(), -1); // -1 - строки с null-terminator

    PGresult *res = execute_params(
        "INSERT INTO user_progress (user_id, sequence_length, success_rate) VALUES ($1, $2, $3)",
        values,
        lengths);

    bool success = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);
    return success;
}

bool DatabaseSync::update_difficulty(uint32_t user_id, uint32_t new_level)
{
    const char *params[2] = {
        std::to_string(new_level).c_str(),
        std::to_string(user_id).c_str()};

    PGresult *res = PQexecParams(get_connection(),
                                 "UPDATE users SET difficulty_level = $1 WHERE id = $2",
                                 2, NULL, params, NULL, NULL, 0);

    bool success = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);
    return success;
}

bool DatabaseSync::update_score(uint32_t user_id, uint32_t score_delta)
{
    const char *params[2] = {
        std::to_string(score_delta).c_str(),
        std::to_string(user_id).c_str()};

    PGresult *res = PQexecParams(get_connection(),
                                 "UPDATE users SET total_score = total_score + $1 WHERE id = $2",
                                 2, NULL, params, NULL, NULL, 0);

    bool success = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);
    return success;
}

int32_t DatabaseSync::get_user_difficulty(uint32_t user_id) const
{
    if (!db_connection || PQstatus(get_connection()) != CONNECTION_OK)
    {
        throw std::runtime_error("Database connection error");
    }

    const char *params[1] = {
        std::to_string(user_id).c_str()};

    PGresult *res = PQexecParams(get_connection(),
                                 "SELECT difficulty_level FROM users WHERE id = $1",
                                 1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        PQclear(res);
        return 0; // EASY по умолчанию
    }

    int32_t difficulty = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return difficulty;
}

std::vector<UserProgress> DatabaseSync::get_user_progress(uint32_t user_id)
{
    // проверка соединения
    if (!db_connection || PQstatus(get_connection()) != CONNECTION_OK)
    {
        throw std::runtime_error("Database connection is not established");
    }

    std::vector<UserProgress> progress;
    const std::string user_id_str = std::to_string(user_id);
    const char *params[1] = {user_id_str.c_str()};

    PGresult *res = PQexecParams(get_connection(),
                                 "SELECT sequence_length, success_rate, training_date FROM user_progress "
                                 "WHERE user_id = $1 ORDER BY training_date DESC",
                                 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        const std::string error_msg = PQerrorMessage(get_connection());
        PQclear(res);
        throw std::runtime_error("Database query failed: " + error_msg);
    }

    try
    {
        const uint32_t rows = PQntuples(res);
        progress.reserve(rows);

        for (std::size_t i{0}; i < rows; ++i)
        {
            UserProgress record;

            const char *length_str = PQgetvalue(res, i, 0);
            const char *rate_str = PQgetvalue(res, i, 1);

            try
            {
                record.sequence_length = std::stoi(length_str);
                record.success_rate = std::stof(rate_str);
            }
            catch (const std::invalid_argument &e)
            {
                throw std::runtime_error("Invalid numeric format in database record");
            }
            catch (const std::out_of_range &e)
            {
                throw std::runtime_error("Numeric value out of range in database record");
            }

            record.training_date = PQgetvalue(res, i, 2);
            progress.push_back(std::move(record));
        }
    }
    catch (...)
    {
        PQclear(res);
        throw;
    }

    PQclear(res);
    return progress;
}
