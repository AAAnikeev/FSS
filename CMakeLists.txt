cmake_minimum_required(VERSION 2.8)

project(FileShareServer)
set(CMAKE_CXX_STANDARD 11)

include_directories(".")
set(SOURCES
    src/main.cpp
    src/db_manager.cpp
)
set(HEADERS src/db_manager.h include/apifss.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} boost_system boost_thread boost_regex stdc++fs pq boost_filesystem)