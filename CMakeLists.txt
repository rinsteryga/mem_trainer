cmake_minimum_required(VERSION 3.15)
project(mem_trainer)

set(CMAKE_C_COMPILER "C:/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/mingw64/bin/g++.exe")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${PROJECT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)