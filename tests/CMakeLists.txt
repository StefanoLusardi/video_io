set(TARGET_NAME video_io_tests)

add_executable(${TARGET_NAME}
    src/main.cpp
    src/test_video_reader.hpp
    src/test_video_reader.cpp
    src/test_video_writer.hpp
    src/test_video_writer.cpp
)

find_package(GTest)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest PRIVATE video_io)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
