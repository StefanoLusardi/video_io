set(TARGET_NAME video_reader_opencv_player)

find_package(OpenCV)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_io 
    PRIVATE opencv::core
    PRIVATE opencv::highgui
)
