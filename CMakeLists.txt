cmake_minimum_required (VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_THREAD_FLAG ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(utils)
add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(tests)
