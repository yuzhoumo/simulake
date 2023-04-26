cmake_minimum_required(VERSION 3.22)

project(simulake
  VERSION 0.0.1
  DESCRIPTION "simulate physics using cellular automata"
  LANGUAGES CXX C)

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

# submodule dependencies
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# shaders
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(COPY ${SHADER_DIR} DESTINATION ${CMAKE_BINARY_DIR})

file(MAKE_DIRECTORY "${SHADER_BUILD_DIR}")
add_custom_target(copy_shaders ALL)

# add a custom command to copy assets when they change
add_custom_command(
    TARGET copy_shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_DIR}" "${SHADER_BUILD_DIR}"
    DEPENDS "${SHADER_DIR}"
    COMMENT "Copying shaders to build directory")

# main executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# defines
target_compile_definitions(${PROJECT_NAME} PRIVATE "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

# opengl
find_package(OpenGL REQUIRED)

# threads
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# threadpool impl
target_include_directories(${PROJECT_NAME} PRIVATE ${VENDOR_DIR}/)

# openmp
if(APPLE)
  # NOTE(vir): toggle next line to install libomp using brew
  # execute_process(COMMAND brew install libomp)
  execute_process (
    COMMAND brew --prefix libomp
    OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
  )
  string(STRIP ${HOMEBREW_LIBOMP_PREFIX} HOMEBREW_LIBOMP_PREFIX)
  message(STATUS "homebrew OpenMP found: " ${HOMEBREW_LIBOMP_PREFIX})

  # set openmp paths
  set(OpenMP_LIBRARIES ${HOMEBREW_LIBOMP_PREFIX}/lib)
  set(OpenMP_INCLUDES ${HOMEBREW_LIBOMP_PREFIX}/include)

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

# glm
add_subdirectory(${VENDOR_DIR}/glm/)
target_include_directories(${PROJECT_NAME} PRIVATE ${VENDOR_DIR}/glm/)

# glad
add_subdirectory(${VENDOR_DIR}/glad/)
target_include_directories(${PROJECT_NAME} PRIVATE ${VENDOR_DIR}/glad/include/)
target_link_libraries(${PROJECT_NAME} glad)

# glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)
add_subdirectory(${VENDOR_DIR}/glfw/)
target_include_directories(${PROJECT_NAME} PRIVATE ${VENDOR_DIR}/glfw/include/)
target_link_libraries(${PROJECT_NAME} glfw)

# benchmark
# include(FetchContent)
# set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
# FetchContent_Declare(benchmark
#   GIT_REPOSITORY https://github.com/google/benchmark.git
#   GIT_TAG v1.7.0)
# FetchContent_MakeAvailable(benchmark)
# target_link_libraries(${PROJECT_NAME} benchmark::benchmark)
