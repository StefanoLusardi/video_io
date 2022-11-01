set(TARGET_NAME video_writer_opencv)

find_package(cppbenchmark)
find_package(OpenCV)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME}
    PRIVATE vio::video_io
    PRIVATE opencv::videoio
    PRIVATE cppbenchmark::cppbenchmark
)
