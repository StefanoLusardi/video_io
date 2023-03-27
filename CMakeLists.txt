cmake_minimum_required (VERSION 3.16)
file(STRINGS VERSION PROJECT_VERSION)
project (video_io 
    LANGUAGES CXX 
    VERSION ${PROJECT_VERSION}
    DESCRIPTION "video_io: video encoder and decoder, written in modern C++"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/video_io"
)

# set(CMAKE_LINK_WHAT_YOU_USE True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_BUILD_TYPE)
cmake_print_variables(CMAKE_GENERATOR)
cmake_print_variables(CMAKE_CXX_COMPILER_ID)
cmake_print_variables(CMAKE_CXX_COMPILER_VERSION)

option(VIDEO_IO_BUILD_TESTS "Build library unit tests" False)
cmake_print_variables(VIDEO_IO_BUILD_TESTS)

option(VIDEO_IO_BUILD_EXAMPLES "Build library examples" True)
cmake_print_variables(VIDEO_IO_BUILD_EXAMPLES)

option(VIDEO_IO_BUILD_SANITIZERS "Build with sanitizers" False)
cmake_print_variables(VIDEO_IO_BUILD_SANITIZERS)

option(VIDEO_IO_BUILD_BENCHMARKS "Build library benchmarks" False)
cmake_print_variables(VIDEO_IO_BUILD_BENCHMARKS)

option(VIDEO_IO_BUILD_DOCS "Build documentation using Doxygen" False)
cmake_print_variables(VIDEO_IO_BUILD_DOCS)

option(VIDEO_IO_INTERNAL_LOGGER "Enable library internal logging" False)
cmake_print_variables(VIDEO_IO_INTERNAL_LOGGER)

include(GNUInstallDirs)
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
