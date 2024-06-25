cmake_minimum_required(VERSION 3.22.1)

file(GLOB parse_module parser/*.cpp)
file(GLOB backend_module backend/*.cpp)

add_definitions(-DDEBUG)
add_executable(dbms	dbms_entry.cpp ../utils/logs.cpp ${parse_module} ${backend_module})