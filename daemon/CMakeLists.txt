add_executable(daemon
	src/main.cpp
        src/communication/ServerSessionConnector.cpp
        src/communication/ServerListener.cpp
		src/management/Directory.cpp
		src/management/InodeList.cpp
		src/management/UsageMap.cpp src/management/DiskException.h)

target_link_libraries(daemon utils)
