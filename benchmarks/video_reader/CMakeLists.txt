set(TARGET_NAME video_reader_opencv)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

find_package(cppbenchmark)
find_package(OpenCV)

target_link_libraries(${TARGET_NAME}
    PRIVATE vio::video_io 
    PRIVATE opencv::videoio
    PRIVATE cppbenchmark::cppbenchmark
)
