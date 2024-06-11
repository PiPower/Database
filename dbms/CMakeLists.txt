cmake_minimum_required(VERSION 3.22.1)

file(GLOB parse_module ./parser/*.cpp)

add_executable(dbms	dbms/dbms_entry.cpp utils/logs.cpp parse_module)