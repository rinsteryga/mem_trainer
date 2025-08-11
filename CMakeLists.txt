cmake_minimum_required(VERSION 3.15)
project(mem_trainer)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    if(NOT DEFINED VCPKG_ROOT)
        set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg" CACHE PATH "VCPKG root directory")
    endif()

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_TARGET_TRIPLET "x64-mingw-static")

    set(INSTALL_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")

    find_path(OPENSSL_INCLUDE_DIR openssl/ssl.h
      PATHS "${INSTALL_DIR}/include"
      REQUIRED
      NO_DEFAULT_PATH
    )

    find_library(OPENSSL_SSL_LIBRARY ssl
      PATHS "${INSTALL_DIR}/lib"
      REQUIRED
      NO_DEFAULT_PATH
    )

    find_library(OPENSSL_CRYPTO_LIBRARY crypto
      PATHS "${INSTALL_DIR}/lib"
      REQUIRED
      NO_DEFAULT_PATH
    )

    find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h
      PATHS "${INSTALL_DIR}/include"
      REQUIRED
      NO_DEFAULT_PATH
    )

    find_library(POSTGRESQL_LIBRARY pq
      PATHS "${INSTALL_DIR}/lib"
      REQUIRED
      NO_DEFAULT_PATH
    )

    add_library(OpenSSL::SSL STATIC IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
      IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${OPENSSL_CRYPTO_LIBRARY}"
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

    if(PostgreSQL_FOUND)
        add_library(PostgreSQL::PQ UNKNOWN IMPORTED)
        set_target_properties(PostgreSQL::PQ PROPERTIES
            IMPORTED_LOCATION "${PostgreSQL_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${PostgreSQL_INCLUDE_DIRS}"
        )
    else()
        set(POSTGRESQL_INCLUDE_DIR "/usr/include/postgresql")
        set(POSTGRESQL_LIBRARY "/usr/lib/x86_64-linux-gnu/libpq.so")
        if(NOT EXISTS "${POSTGRESQL_LIBRARY}")
            message(FATAL_ERROR "PostgreSQL library not found at: ${POSTGRESQL_LIBRARY}")
        endif()

        add_library(PostgreSQL::PQ UNKNOWN IMPORTED)
        set_target_properties(PostgreSQL::PQ PROPERTIES
            IMPORTED_LOCATION "${POSTGRESQL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${POSTGRESQL_INCLUDE_DIR}"
        )
    endif()
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
)

if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${POSTGRESQL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        PostgreSQL::PQ
        OpenSSL::SSL
    )

    target_link_options(${PROJECT_NAME} PRIVATE 
        -static
        -static-libgcc 
        -static-libstdc++
    )
else()
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${PostgreSQL_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        PostgreSQL::PQ
        OpenSSL::SSL
    )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/config.ini")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/config.ini"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying config.ini to output directory"
    )
endif()

message(STATUS "OpenSSL includes: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libraries: ${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}")
message(STATUS "PostgreSQL library: ${POSTGRESQL_LIBRARY}")
message(STATUS "PostgreSQL includes: ${POSTGRESQL_INCLUDE_DIR}")