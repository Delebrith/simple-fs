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


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests/auto")
add_executable(create_succesfully-create-file src/auto/create_succesfully-create-file.cpp)
add_executable(create_no-read-access-on-path src/auto/create_no-read-access-on-path.cpp)
add_executable(create_no-write-access-on-directory src/auto/create_no-write-access-on-directory.cpp)
add_executable(create_path-not-found src/auto/create_path-not-found.cpp)
add_executable(create_slash-or-dots src/auto/create_slash-or-dots.cpp)
add_executable(create_not-in-dir src/auto/create_not-in-dir.cpp)
add_executable(chmod_take-away-write src/auto/chmod_take-away-write.cpp)
add_executable(chmod_take-away-read src/auto/chmod_take-away-read.cpp)
add_executable(chmod_file-in-usage src/auto/chmod_file-in-usage.cpp)
add_executable(read_write_succesfully-change-file src/auto/read_write_succesfully-change-file.cpp)
add_executable(read_write_bad-open-access src/auto/read_write_bad-open-access.cpp)
add_executable(mkdir_succesfully-create-dir src/auto/mkdir_succesfully-create-dir.cpp)
add_executable(mkdir_no-read-access-on-path src/auto/mkdir_no-read-access-on-path.cpp)
add_executable(mkdir_no-write-access-on-directory src/auto/mkdir_no-write-access-on-directory.cpp)
add_executable(mkdir_path-not-found src/auto/mkdir_path-not-found.cpp)
add_executable(mkdir_slash-or-dots src/auto/mkdir_slash-or-dots.cpp)
add_executable(mkdir_not-in-dir src/auto/mkdir_not-in-dir.cpp)
add_executable(unlink_succesfully-delete-file src/auto/unlink_succesfully-delete-file.cpp)
add_executable(unlink_fail-on-used-file src/auto/unlink_fail-on-used-file.cpp)
add_executable(unlink_no-write-access src/auto/unlink_no-write-access.cpp)
add_executable(unlink_path-not-found src/auto/unlink_path-not-found.cpp)
add_executable(unlink_shalh-or-dots src/auto/unlink_slash-or-dots.cpp)

target_link_libraries(create_succesfully-create-file lib)
target_link_libraries(create_no-read-access-on-path lib)
target_link_libraries(create_no-write-access-on-directory lib)
target_link_libraries(create_path-not-found lib)
target_link_libraries(create_slash-or-dots lib)
target_link_libraries(create_not-in-dir lib)
target_link_libraries(chmod_take-away-write lib)
target_link_libraries(chmod_take-away-read lib)
target_link_libraries(chmod_file-in-usage lib)
target_link_libraries(read_write_succesfully-change-file lib)
target_link_libraries(read_write_bad-open-access lib)
target_link_libraries(mkdir_succesfully-create-dir lib)
target_link_libraries(mkdir_no-read-access-on-path lib)
target_link_libraries(mkdir_no-write-access-on-directory lib)
target_link_libraries(mkdir_path-not-found lib)
target_link_libraries(mkdir_slash-or-dots lib)
target_link_libraries(mkdir_not-in-dir lib)
target_link_libraries(unlink_succesfully-delete-file lib)
target_link_libraries(unlink_fail-on-used-file lib)
target_link_libraries(unlink_no-write-access lib)
target_link_libraries(unlink_path-not-found lib)
target_link_libraries(unlink_shalh-or-dots lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests")

add_executable(mp src/mp/mp.cpp)
target_link_libraries(mp lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
