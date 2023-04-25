cmake_minimum_required(VERSION 3.22)

project(simulake
    VERSION 0.1
    DESCRIPTION "simulate physics using cellular automata"
    LANGUAGES CXX)

# use fetchcontent for dependencies
include(FetchContent)

# source files
file(GLOB_RECURSE SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/*.c
  ${CMAKE_SOURCE_DIR}/src/*.cpp)

# header files
file(GLOB_RECURSE HEADER_FILES
  ${CMAKE_SOURCE_DIR}/src/*.h
  ${CMAKE_SOURCE_DIR}/src/*.hpp)

# main executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# threads
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# benchmark
# set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
# FetchContent_Declare(benchmark
#   GIT_REPOSITORY https://github.com/google/benchmark.git
#   GIT_TAG v1.7.0)
# FetchContent_MakeAvailable(benchmark)
# target_link_libraries(${PROJECT_NAME} benchmark::benchmark)
