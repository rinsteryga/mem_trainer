#pragma once

#include "Value.hpp"

class TaskGenerator{
public:
    ~TaskGenerator();
private:
    Value<int>* value_;
};
