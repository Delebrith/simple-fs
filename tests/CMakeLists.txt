configure_file(src/run_tests.sh "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/run_tests.sh" COPYONLY)

add_executable(mkdir src/manual/mkdir.cpp)
add_executable(read src/manual/read.cpp)
add_executable(write src/manual/write.cpp)
add_executable(chmod src/manual/chmod.cpp)
add_executable(create src/manual/create.cpp)

target_link_libraries(mkdir lib)
target_link_libraries(read lib)
target_link_libraries(write lib)
target_link_libraries(chmod lib)
target_link_libraries(create lib)

