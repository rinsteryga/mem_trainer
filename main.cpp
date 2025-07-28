#include "MainLoop.hpp"

#include <thread>
#include <chrono>

int main(int argc, char** argv){
    while (true)
    {
        print_auth();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        break;
    }
    return 0;
}