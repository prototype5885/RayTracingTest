cmake_minimum_required(VERSION 3.25)
project(RayCastingGaem)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_NAME RayCastingGaem.exe)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if(UNIX)
  find_package(SDL2 REQUIRED)
elseif(WIN32)
endif()

if(UNIX)
  target_link_libraries(${EXECUTABLE_NAME} -lm ${SDL2_LIBRARIES})
elseif(WIN32)
add_executable(${EXECUTABLE_NAME} WIN32 
        src/Main.cpp
        )

  target_link_libraries(${EXECUTABLE_NAME} -lmingw32 -lSDL2main -lSDL2 -static-libstdc++)
endif()


