-- Создание таблицы users
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    password VARCHAR(100) NOT NULL,
    difficulty_level INTEGER NOT NULL DEFAULT 0,
    total_score INTEGER NOT NULL DEFAULT 0,
    last_session TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Создание таблицы user_progress
CREATE TABLE user_progress (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    sequence_length INTEGER NOT NULL,
    success_rate DOUBLE PRECISION NOT NULL,
    training_date TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    -- Внешний ключ для связи с таблицей users
    CONSTRAINT fk_user
        FOREIGN KEY(user_id) 
        REFERENCES users(id)
        ON DELETE CASCADE
);

-- Создание индексов для ускорения запросов
CREATE INDEX idx_user_progress_user_id ON user_progress(user_id);
CREATE INDEX idx_user_progress_training_date ON user_progress(training_date);
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_total_score ON users(total_score DESC);

-- Комментарии к таблицам и полям
COMMENT ON TABLE users IS 'Таблица пользователей системы';
COMMENT ON COLUMN users.difficulty_level IS 'Уровень сложности: 0=EASY, 1=MEDIUM, 2=HARD';
COMMENT ON COLUMN users.total_score IS 'Общий счет пользователя';

COMMENT ON TABLE user_progress IS 'История тренировок пользователей';
COMMENT ON COLUMN user_progress.success_rate IS 'Процент успешного выполнения (0.0-1.0)';
