cmake_minimum_required(VERSION 3.22.1)

file(GLOB parse_module parser/*.cpp)
file(GLOB backend_module backend/*.cpp)
file(GLOB algorithms algorithms/*.cpp)

option(REMOTE_CLIENT "REMOTE_CLIENT" OFF)

if(REMOTE_CLIENT)
	add_definitions(-DREMOTE_CLIENT)
endif()

add_executable(dbms	dbms_entry.cpp ${parse_module} ${backend_module} ${algorithms})