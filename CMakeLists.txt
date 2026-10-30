cmake_minimum_required(VERSION 3.20)

if(APPLE AND NOT DEFINED CMAKE_CXX_COMPILER AND EXISTS /opt/homebrew/opt/llvm/bin/clang++)
  set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++ CACHE FILEPATH "C++ compiler")
endif()

project(Assignment1 LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory and run CMake from there.")
endif()

option(TSAN "Build with ThreadSanitizer" OFF)
option(WITH_OPENMP "Build the OpenMP lock and barrier baselines" ON)
option(WITH_MPI "Build the Part 4 MPI programs" ON)

include(cmake/CompilerOptions.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)

file(GLOB common_sources CONFIGURE_DEPENDS src/common/*.cpp)
add_library(common STATIC ${common_sources})
target_include_directories(common PUBLIC include src)
target_compile_definitions(common PUBLIC CACHE_LINE=${CACHE_LINE})
target_link_libraries(common PUBLIC Threads::Threads)

if(WITH_OPENMP AND NOT TSAN)
  find_package(OpenMP COMPONENTS CXX)
  if(OpenMP_CXX_FOUND)
    target_link_libraries(common PUBLIC OpenMP::OpenMP_CXX)
  else()
    message(STATUS "OpenMP not found: omp lock and barrier baselines are left out")
  endif()
endif()

file(GLOB bench_sources CONFIGURE_DEPENDS src/bench_*.cpp)
foreach(source ${bench_sources})
  get_filename_component(name ${source} NAME_WE)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE common)
endforeach()

enable_testing()
add_subdirectory(test)

if(WITH_MPI)
  add_subdirectory(mpi)
endif()
