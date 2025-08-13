# Memory Trainer - README

## 🧠 Project Description  
Memory Trainer is a console application designed to improve memory skills through sequence recall challenges. Users memorize and reproduce sequences of numbers, symbols, and words while tracking their progress and competing on leaderboards.

## ✨ Key Features

### 🔐 Authentication
- User registration with username/password
- Secure login system
- Automatic session tracking

### 🎮 Training Modes
- **3 Difficulty Levels**:
  - Easy (short sequences, mixed types)
  - Medium (longer sequences, numbers/words) 
  - Hard (complex sequences, less time)

### 🧩 Sequence Generation
- Randomly generated sequences containing:
  - Integers (16/32-bit)
  - Floating-point numbers
  - Alphabet characters
  - Common words

### 📊 Progress Tracking
- Detailed training history
- Success rate statistics
- Adaptive difficulty adjustment
- Score calculation based on performance

### 🏆 Competitive Elements
- Global leaderboard (top 10 players)
- Score-based ranking system
- Personal progress visualization

## 🛠 Technical Implementation

### 📦 Core Technologies
- **Language**: C++20
- **Database**: PostgreSQL
- **Dependencies**:
  - libpq (PostgreSQL client)
  - OpenSSL (for secure connections)

### 🏗 System Architecture
- Modular design with separated components:
  - Database layer (PostgreSQL interface)
  - Game logic (sequence generation/scoring)
  - UI layer (console interface)

## 🚀 Getting Started

### Prerequisites
- PostgreSQL server (v12+)
- C++20 compatible compiler
- libpq development libraries

### Installation
1. Clone repository
2. Configure `config.ini` with database credentials
3. Run `install_db.sql` to initialize database
4. Build with CMake

## 📜 License
MIT License - Free for educational and personal use

---

> "Improve your memory while competing with others!"