cmake_minimum_required(VERSION 3.15)
project(mem_trainer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    if(NOT DEFINED VCPKG_ROOT)
        set(VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "VCPKG root directory")
        message(STATUS "VCPKG_ROOT: ${VCPKG_ROOT}")
    endif()

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "Vcpkg triplet")

    set(INSTALL_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
    file(TO_CMAKE_PATH "${INSTALL_DIR}" INSTALL_DIR)

    find_path(OPENSSL_INCLUDE_DIR openssl/ssl.h
        PATHS "${INSTALL_DIR}/include"
        NO_DEFAULT_PATH
    )
    
    if(NOT OPENSSL_INCLUDE_DIR)
        find_path(OPENSSL_INCLUDE_DIR openssl/ssl.h
            PATHS "${INSTALL_DIR}/include/openssl"
            NO_DEFAULT_PATH
        )
    endif()

    if(NOT OPENSSL_INCLUDE_DIR)
        message(FATAL_ERROR "OpenSSL headers not found in ${INSTALL_DIR}")
    endif()

    find_library(OPENSSL_SSL_LIBRARY ssl
        PATHS "${INSTALL_DIR}/lib"
        NO_DEFAULT_PATH
    )
    find_library(OPENSSL_CRYPTO_LIBRARY crypto
        PATHS "${INSTALL_DIR}/lib"
        NO_DEFAULT_PATH
    )

    add_library(OpenSSL::Crypto STATIC IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )

    add_library(OpenSSL::SSL STATIC IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OpenSSL::Crypto"
    )

    find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h
        PATHS 
            "${INSTALL_DIR}/include"
            "${INSTALL_DIR}/include/postgresql"
        NO_DEFAULT_PATH
    )
    
    find_library(POSTGRESQL_LIBRARY pq
        PATHS "${INSTALL_DIR}/lib"
        NO_DEFAULT_PATH
    )

    add_library(PostgreSQL::PQ STATIC IMPORTED)
    set_target_properties(PostgreSQL::PQ PROPERTIES
        IMPORTED_LOCATION "${POSTGRESQL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${POSTGRESQL_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "ws2_32;secur32;OpenSSL::SSL"
    )
else()
    find_package(OpenSSL REQUIRED)
    find_package(PostgreSQL REQUIRED)
    add_library(PostgreSQL::PQ UNKNOWN IMPORTED)
    set_target_properties(PostgreSQL::PQ PROPERTIES
        IMPORTED_LOCATION "${PostgreSQL_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${PostgreSQL_INCLUDE_DIRS}"
    )
endif()

set(SOURCES
    src/MainLoop.cpp
    src/DatabaseSync.cpp
    src/Menu.cpp
    src/TaskGenerator.cpp
    src/RandomGenerators.cpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${POSTGRESQL_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    PostgreSQL::PQ
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE 
        -static
        -static-libgcc 
        -static-libstdc++
    )
endif()