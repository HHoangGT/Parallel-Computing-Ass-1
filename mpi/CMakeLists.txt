find_package(MPI COMPONENTS CXX)
if(NOT MPI_CXX_FOUND)
  message(STATUS "MPI not found: mpi_counter, mpi_lock and mpi_barrier are left out")
  return()
endif()

add_library(mpi_common STATIC mpi_common.cpp)
target_include_directories(mpi_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mpi_common PUBLIC common MPI::MPI_CXX)

foreach(name mpi_counter mpi_lock mpi_barrier)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE mpi_common)
endforeach()
