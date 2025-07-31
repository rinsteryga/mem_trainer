#pragma once

#include <string>
#include <array>

class NumberGenerator
{
public:
    static uint16_t generate_uint16() noexcept;
    static uint32_t generate_uint32() noexcept;
    static float generate_float() noexcept;

private:
    template <typename T>
    static T generate() noexcept;
};

class SymbolGenerator
{
public:
    static char generate_char() noexcept;
    static std::string generate_string(size_t length) noexcept;

private:
    static constexpr std::array<char, 52> symbols = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z'};
};

class WordGenerator
{
public:
    static std::string generate_word() noexcept;

private:
    static constexpr std::array<const char *, 100> words = {
        "treadmill", "dumbbell", "barbell", "squat", "deadlift",
        "benchpress", "pullup", "pushup", "plank", "crunch",
        "kettlebell", "rower", "elliptical", "protein", "creatine",
        "gymbag", "headband", "wristwrap", "lift", "rep",
        "set", "cardio", "flex", "pump", "spotter",
        "variable", "function", "loop", "array", "pointer",
        "class", "object", "template", "lambda", "algorithm",
        "compile", "debug", "syntax", "boolean", "integer",
        "string", "vector", "hash", "queue", "stack",
        "binary", "recursion", "iterator", "namespace", "inheritance",
        "apple", "banana", "cherry", "date", "elderberry",
        "fig", "grape", "honeydew", "kiwi", "lemon",
        "mango", "nectarine", "orange", "peach", "quince",
        "raspberry", "strawberry", "tangerine", "ugli", "vanilla",
        "watermelon", "xigua", "yam", "zucchini", "almond",
        "bread", "cheese", "dough", "egg", "flour",
        "garlic", "honey", "ice", "jam", "kale",
        "lentil", "milk", "nut", "olive", "pepper",
        "quinoa", "rice", "salt", "tomato", "vinegar",
        "wheat", "yogurt", "zest", "butter", "cinnamon"};
};
