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
option(SEQ_CST "Ablation: make every memory order in locks, barrier and counters seq_cst" OFF)

include(cmake/CompilerOptions.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Threads REQUIRED)

file(GLOB common_sources CONFIGURE_DEPENDS src/common/*.cpp)
add_library(common STATIC ${common_sources})
target_include_directories(common PUBLIC include src)
target_compile_definitions(common PUBLIC CACHE_LINE=${CACHE_LINE})
if(SEQ_CST)
  target_compile_definitions(common PUBLIC ASM1_SEQ_CST)
  message(STATUS "SEQ_CST ablation: all memory orders are seq_cst")
endif()
target_link_libraries(common PUBLIC Threads::Threads)

if(WITH_OPENMP AND NOT TSAN)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    # Apple clang has no -fopenmp driver flag; point FindOpenMP at Homebrew's libomp.
    execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE libomp_prefix RESULT_VARIABLE brew_status
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(brew_status EQUAL 0 AND EXISTS "${libomp_prefix}/lib/libomp.dylib")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${libomp_prefix}/include")
      set(OpenMP_CXX_LIB_NAMES omp)
      set(OpenMP_omp_LIBRARY "${libomp_prefix}/lib/libomp.dylib")
    endif()
  endif()
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
