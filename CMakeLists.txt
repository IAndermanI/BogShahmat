cmake_minimum_required(VERSION 3.24)
project(BogShahmat)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-march=native)

add_executable(BogShahmat main.cpp bitboards_and_constants.h move_logic.h zobrist.h print_boards.h move_implementation.h perft.h print_moves.h evaluation.h)
