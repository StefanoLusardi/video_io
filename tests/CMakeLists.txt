set(TARGET_NAME video_io_tests)

add_executable(${TARGET_NAME}
    test/main.cpp
    test/test_video_reader.cpp
    test/test_video_writer.cpp
)

find_package(GTest)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest)
target_include_directories(${TARGET_NAME} PRIVATE include)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
