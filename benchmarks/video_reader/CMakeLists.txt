set(TARGET_NAME video_reader_opencv)

# REQUIRES cppbenchmark/cci.20201029
# REQUIRES opencv/4.5.5

find_package(cppbenchmark)
find_package(OpenCV)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_reader 
    PRIVATE opencv::core
    PRIVATE opencv::videoio
    PRIVATE cppbenchmark::cppbenchmark
)
