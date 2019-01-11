add_executable(daemon
	src/main.cpp
        src/communication/ServerSessionConnector.cpp
        src/communication/ServerListener.cpp
		src/management/Directory.cpp
		src/management/InodeList.cpp
		src/management/UsageMap.cpp
		src/management/DiskException.h
		src/management/InodeDirectoryEntry.cpp
	src/logging/Log.cpp src/diskfunctions/DiskOperations.cpp src/diskfunctions/DiskOperations.h)

find_package(Threads REQUIRED)

target_link_libraries(daemon 
		utils
		Threads::Threads)
