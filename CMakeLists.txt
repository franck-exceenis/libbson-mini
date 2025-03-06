cmake_minimum_required(VERSION 3.20)

project("bson")

add_library("${PROJECT_NAME}" STATIC "src/bson.c")
target_include_directories("${PROJECT_NAME}" PUBLIC "src")

add_library("${PROJECT_NAME}++" STATIC "src/bson.cpp")
target_include_directories("${PROJECT_NAME}++" PUBLIC "src")
target_link_libraries("${PROJECT_NAME}++" PUBLIC "${PROJECT_NAME}")

add_executable("bson_reader" "bson_reader.c")
target_link_libraries("bson_reader" "${PROJECT_NAME}")
