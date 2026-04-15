cmake_minimum_required(VERSION 3.16)
project(crewgle_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(crewgle_backend
        src/main.cpp
)

find_package(Drogon CONFIG REQUIRED)
target_link_libraries(crewgle_backend PRIVATE Drogon::Drogon)