cmake_minimum_required(VERSION 3.16)
project(aoc-2024)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CPM.cmake)

CPMAddPackage("gh:gabime/spdlog#v1.15.0")

function(add_day day_name)
    add_executable(${day_name} src/${day_name}.cpp)
    target_compile_features(${day_name} PRIVATE cxx_std_17)
    target_link_libraries(${day_name} PRIVATE spdlog)
endfunction()

add_day(day1)
add_day(day2)
add_day(day3)
