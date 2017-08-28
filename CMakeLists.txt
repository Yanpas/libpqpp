cmake_minimum_required(VERSION 2.8)
project(pqpp)
add_library(${PROJECT_NAME} STATIC src/pqpp.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} pq)

add_executable(pqtest test_exe/test.cpp)
target_link_libraries(pqtest pqpp)
