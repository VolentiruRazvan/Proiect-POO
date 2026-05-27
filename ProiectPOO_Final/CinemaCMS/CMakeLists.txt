cmake_minimum_required(VERSION 3.8)
project("CinemaCMS")

# Forțăm C++17 pentru compatibilitatea OOP și nlohmann/json
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Curses REQUIRED)

# Linkăm fișierele modularizate
add_executable(CinemaCMS main.cpp CinemaCMS.cpp)
target_link_libraries(CinemaCMS ${CURSES_LIBRARIES})