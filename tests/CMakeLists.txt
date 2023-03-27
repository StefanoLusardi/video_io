list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/tests)
find_package(GTest)
include(GoogleTest)

set(TARGET_SOURCES
    src/main.cpp
    src/test_video_reader.hpp
    src/test_video_reader.cpp
    src/test_video_writer.hpp
    src/test_video_writer.cpp
)

set(TARGET_NAME video_io_tests)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PUBLIC ${TARGET_SOURCES})
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest PRIVATE vio::video_io)
gtest_discover_tests(${TARGET_NAME})
