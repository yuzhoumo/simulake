cmake_minimum_required(VERSION 3.22)

project(simulake
  VERSION 0.0.1
  DESCRIPTION "simulate physics using cellular automata"
  LANGUAGES CXX C)

# cmake config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# project header files
set(HEADER_FILES
  ${SRC_DIR}/include/bs_threadpool.hpp)

# project source files
set(SOURCE_FILES
  ${SRC_DIR}/main.cpp)

# required packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# benchmark
include(FetchContent)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
FetchContent_Declare(benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.7.0)
FetchContent_MakeAvailable(benchmark)

# glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)
add_subdirectory(${VENDOR_DIR}/glfw/)

# glad
add_subdirectory(${VENDOR_DIR}/glad/)

# glm
add_subdirectory(${VENDOR_DIR}/glm/)

# set executable and link
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${SRC_DIR}
  ${VENDOR_DIR}/glad/include/
  ${VENDOR_DIR}/glfw/include/
  ${VENDOR_DIR}/glm/)

target_link_libraries(${PROJECT_NAME}
  benchmark::benchmark
  Threads::Threads
  glad
  glfw)
