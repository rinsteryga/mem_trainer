# Database Schema Documentation

## Tables Overview

### 1. `users` Table
Stores user account information and game statistics.

**Columns:**
- `id` (SERIAL, PRIMARY KEY): Unique user identifier
- `username` (VARCHAR(50), NOT NULL): User's login name
- `password` (VARCHAR(100), NOT NULL): Encrypted password
- `difficulty_level` (INTEGER, DEFAULT 0): Current difficulty setting (0=EASY, 1=MEDIUM, 2=HARD)
- `total_score` (INTEGER, DEFAULT 0): Accumulated game score
- `last_session` (TIMESTAMP): Last active session timestamp

**Indexes:**
- Primary key on `id`
- `idx_users_username`: Optimizes username lookups
- `idx_users_total_score`: Sorts by score (DESC) for leaderboards

### 2. `user_progress` Table
Tracks training session history and performance metrics.

**Columns:**
- `id` (SERIAL, PRIMARY KEY): Progress record identifier
- `user_id` (INTEGER, NOT NULL): Reference to users.id
- `sequence_length` (INTEGER, NOT NULL): Training sequence length
- `success_rate` (DOUBLE PRECISION, NOT NULL): Completion accuracy (0.0-1.0)
- `training_date` (TIMESTAMP): When the session occurred

**Relationships:**
- Foreign key `fk_user` linking to `users.id` with CASCADE delete

**Indexes:**
- Primary key on `id`
- `idx_user_progress_user_id`: Speeds up user history queries
- `idx_user_progress_training_date`: Optimizes date-based sorting

## Configuration

Database connection parameters are stored in `config.ini`:

```ini
[database]
host=localhost
port=5432
dbname=your_db
user=your_user
password=your_password