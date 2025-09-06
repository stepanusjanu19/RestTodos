cmake_minimum_required(VERSION 3.28)
project(simple_rest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

#include(FetchContent)

#FetchContent_Declare(
#        httplib
#        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
#        GIT_TAG v0.10.0
#)
#
#FetchContent_Declare(
#        nlohmann_json
#        GIT_REPOSITORY https://github.com/nlohmann/json.git
#        GIT_TAG v3.11.2
#)

#FetchContent_MakeAvailable(httplib nlohmann_json)

add_executable(simple_rest main.cpp)

#target_include_directories(simple_rest PRIVATE ${httplib_SOURCE_DIR})
#target_link_libraries(simple_rest PRIVATE nlohmann_json::nlohmann_json pthread)