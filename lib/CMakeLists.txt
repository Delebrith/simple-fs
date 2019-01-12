add_library(lib
	src/simplefs.cpp
	src/ClientConnector.cpp)

target_link_libraries(lib utils)
