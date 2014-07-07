cmake_minimum_required(VERSION 2.8.8)

project(FileWatcher)

set(FILEWATCHER_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../")

set(FILEWATCHER_SRC	FileWatcher.cpp)

if(BUILD_LINUX)
	set(FILEWATCHER_SRC ${FILEWATCHER_SRC} FileWatcherLinux.cpp)
endif(BUILD_LINUX)

include_directories(${FILEWATCHER_INCLUDES})
add_library(FileWatcher STATIC ${FILEWATCHER_SRC})
target_include_directories(FileWatcher PUBLIC ${FILEWATCHER_INCLUDES})
