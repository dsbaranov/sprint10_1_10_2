cmake_minimum_required(VERSION 3.0.0)
project(sprint10_1_10_2 VERSION 0.1.0 LANGUAGES C CXX)
add_executable(sprint10_1_10_2 main.cpp log_duration.h json.cpp json.h)
target_compile_options(sprint10_1_10_2 PRIVATE -Wall -Wextra -Wpedantic -Werror)
