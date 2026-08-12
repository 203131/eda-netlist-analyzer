cmake_minimum_required(VERSION 3.12)
project(NetlistAnalyzer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)
file(GLOB SOURCES "src/*.cpp")

add_executable(netlist_analyzer ${SOURCES})