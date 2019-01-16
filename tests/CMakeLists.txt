configure_file(src/run_tests.sh "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/run_tests.sh" COPYONLY)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests/manual")

add_executable(mkdir src/manual/mkdir.cpp)
add_executable(read src/manual/read.cpp)
add_executable(write src/manual/write.cpp)
add_executable(chmod src/manual/chmod.cpp)
add_executable(create src/manual/create.cpp)
add_executable(ls src/manual/ls.cpp)
add_executable(unlink src/manual/unlink.cpp)

target_link_libraries(mkdir lib)
target_link_libraries(read lib)
target_link_libraries(write lib)
target_link_libraries(chmod lib)
target_link_libraries(create lib)
target_link_libraries(ls lib)
target_link_libraries(unlink lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
