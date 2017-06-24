cmake_minimum_required(VERSION 3.8)

set(SOURCES unigine.cpp)

set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

project(unigine_test_task)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
