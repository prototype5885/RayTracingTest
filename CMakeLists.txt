cmake_minimum_required(VERSION 3.25)
project(RayTracing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_NAME RayTracing)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_executable(${EXECUTABLE_NAME} src/Main.cpp)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE -lmingw32 -lSDL2main -lSDL2  -static-libstdc++)