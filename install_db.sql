-- Create users table
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    password VARCHAR(100) NOT NULL,
    difficulty_level INTEGER NOT NULL DEFAULT 0,
    total_score INTEGER NOT NULL DEFAULT 0,
    last_session TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Create user_progress table
CREATE TABLE user_progress (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    sequence_length INTEGER NOT NULL,
    success_rate DOUBLE PRECISION NOT NULL,
    training_date TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    -- Foreign key relationship with users table
    CONSTRAINT fk_user
        FOREIGN KEY(user_id) 
        REFERENCES users(id)
        ON DELETE CASCADE
);

-- Create indexes for query optimization
CREATE INDEX idx_user_progress_user_id ON user_progress(user_id);
CREATE INDEX idx_user_progress_training_date ON user_progress(training_date);
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_total_score ON users(total_score DESC);

-- Table and column comments
COMMENT ON TABLE users IS 'System users table';
COMMENT ON COLUMN users.difficulty_level IS 'Difficulty level: 0=EASY, 1=MEDIUM, 2=HARD';
COMMENT ON COLUMN users.total_score IS 'User''s total score';

COMMENT ON TABLE user_progress IS 'User training history';
COMMENT ON COLUMN user_progress.success_rate IS 'Success completion percentage (0.0-1.0)';
