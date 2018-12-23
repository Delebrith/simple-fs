add_executable(daemon
	src/main.cpp
	src/ServerSessionConnector.cpp
	src/ServerListener.cpp)

target_link_libraries(daemon utils)
