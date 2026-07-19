cmake_minimum_required(VERSION 3.20)
project(crewgle_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(crewgle_backend
    src/main.cpp
    src/Database.cpp
    src/Http.cpp
    src/Security.cpp
    src/Schema.cpp
    src/Services.cpp
    src/Routes.cpp
    src/Email.cpp
)

target_include_directories(crewgle_backend PRIVATE include)

find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(crewgle_backend
    PRIVATE
    Drogon::Drogon
    OpenSSL::Crypto
    sqlite3
)
