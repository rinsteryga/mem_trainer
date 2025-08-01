cmake_minimum_required(VERSION 3.15)
project(mem_trainer)

set(VCPKG_ROOT "C:/vcpkg-2025.07.25")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-mingw-static")

set(CMAKE_C_COMPILER "C:/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/mingw64/bin/g++.exe")
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

set(INSTALL_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")

set(OPENSSL_INCLUDE_DIR "${INSTALL_DIR}/include")
set(OPENSSL_CRYPTO_LIBRARY "${INSTALL_DIR}/lib/libcrypto.a")
set(OPENSSL_SSL_LIBRARY "${INSTALL_DIR}/lib/libssl.a")

set(POSTGRESQL_INCLUDE_DIR "${INSTALL_DIR}/include/postgresql")
set(POSTGRESQL_LIBRARY "${INSTALL_DIR}/lib/libpq.a")

add_library(OpenSSL::SSL STATIC IMPORTED)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
)

add_library(PostgreSQL::PQ STATIC IMPORTED)
set_target_properties(PostgreSQL::PQ PROPERTIES
    IMPORTED_LOCATION "${POSTGRESQL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${POSTGRESQL_INCLUDE_DIR}"
)

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES} ${PROJECT_SOURCE_DIR}/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -pthread
    ${CMAKE_THREAD_LIBS_INIT}
    PostgreSQL::PQ
    OpenSSL::SSL
)