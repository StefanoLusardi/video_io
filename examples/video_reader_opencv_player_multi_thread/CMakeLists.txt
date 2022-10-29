set(TARGET_NAME video_reader_opencv_player_multi_thread)

find_package(OpenCV)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE vio::video_io-static
    PRIVATE opencv::highgui
)
