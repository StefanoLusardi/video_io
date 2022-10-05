cmake_minimum_required (VERSION 3.16)

file(STRINGS VERSION PROJECT_VERSION)
project (video_io 
    LANGUAGES CXX 
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "video_io: video encoder and decoder, written in modern C++"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/video_io"
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_BINARY_DIR}/modules)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(GNUInstallDirs)

# option(VIDEO_IO_BUILD_SHARED "Build shared library" ON)
option(VIDEO_IO_BUILD_TESTS "Build library unit tests" ON)
option(VIDEO_IO_BUILD_EXAMPLES "Build library examples" ON)
option(VIDEO_IO_BUILD_SANITIZERS "Build with sanitizers (address, undefined, memory, thread)" OFF)
option(VIDEO_IO_BUILD_BENCHMARKS "Build library benchmarks" OFF)
option(VIDEO_IO_BUILD_DOCS "Build documentation using Doxygen" OFF)
option(VIDEO_IO_INTERNAL_LOGGER "Enable library internal logging" OFF)

add_subdirectory(video_io)

if(${VIDEO_IO_BUILD_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

if(${VIDEO_IO_BUILD_EXAMPLES})
	add_subdirectory(examples)
endif()

if(${VIDEO_IO_BUILD_BENCHMARKS})
	add_subdirectory(benchmarks)
endif()

if(${VIDEO_IO_BUILD_DOCS})
	add_subdirectory(docs)
endif()
