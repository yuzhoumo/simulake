cmake_minimum_required(VERSION 3.22)

project(simulake
    VERSION 0.1
    DESCRIPTION "simulate physics using cellular automata"
    LANGUAGES C CXX)

# options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# threads, openmp
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# openmp
# get homebrew openmp prefix
execute_process (
  COMMAND brew --prefix libomp
  OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
)
string(STRIP ${HOMEBREW_LIBOMP_PREFIX} HOMEBREW_LIBOMP_PREFIX)
message(STATUS "homebrew OpenMP found: " ${HOMEBREW_LIBOMP_PREFIX})

# set openmp paths
set(OpenMP_LIBRARIES ${HOMEBREW_LIBOMP_PREFIX}/lib)
set(OpenMP_INCLUDES ${HOMEBREW_LIBOMP_PREFIX}/include)

# set compiler flags
if(APPLE)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(OpenMP_C "${CMAKE_C_COMPILER}" CACHE STRING "" FORCE)
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -lomp -Wno-unused-command-line-argument" CACHE STRING "" FORCE)
    set(OpenMP_C_LIB_NAMES "omp" CACHE STRING "" FORCE)
    set(OpenMP_omp_LIBRARY ${OpenMP_C_LIB_NAMES} CACHE STRING "" FORCE)
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OpenMP_CXX "${CMAKE_CXX_COMPILER}" CACHE STRING "" FORCE)
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -lomp -Wno-unused-command-line-argument" CACHE STRING "" FORCE)
    set(OpenMP_CXX_LIB_NAMES "omp" CACHE STRING "" FORCE)
    set(OpenMP_omp_LIBRARY ${OpenMP_CXX_LIB_NAMES} CACHE STRING "" FORCE)
  endif()
endif()

find_package(OpenMP REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenMP_INCLUDES})
target_link_directories(${PROJECT_NAME} PRIVATE ${OpenMP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)

# benchmark
# include(FetchContent)
# set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
# FetchContent_Declare(benchmark
#   GIT_REPOSITORY https://github.com/google/benchmark.git
#   GIT_TAG v1.7.0)
# FetchContent_MakeAvailable(benchmark)
# target_link_libraries(${PROJECT_NAME} benchmark::benchmark)
