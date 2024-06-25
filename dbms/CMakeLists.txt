project(Database_Management_System)
cmake_minimum_required(VERSION 3.22.1)

file(GLOB parse_module parser/*.cpp)
file(GLOB backend_module backend/*.cpp)

option(REMOTE_CLIENT "REMOTE_CLIENT" OFF)

if(REMOTE_CLIENT)
	add_definitions(-DREMOTE_CLIENT)
endif()

add_executable(dbms	dbms_entry.cpp ../utils/logs.cpp ${parse_module} ${backend_module})