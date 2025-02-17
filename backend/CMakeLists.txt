cmake_minimum_required(VERSION 3.10)
project(process_scheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)
find_package(Boost REQUIRED)

add_executable(process_scheduler main.cpp)
target_link_libraries(process_scheduler PRIVATE Crow::Crow)