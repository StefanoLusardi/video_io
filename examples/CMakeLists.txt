list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/examples)

# add_subdirectory(comparison_opencv_video_io) 

# add_subdirectory(video_reader_opencv_player)

# add_subdirectory(video_reader_imgui_player)
# add_subdirectory(video_reader_imgui_player_multi_thread)

# add_subdirectory(video_reader_opengl_player)
# add_subdirectory(video_reader_opengl_player_multi_thread)

add_subdirectory(video_reader_simple_decode)
add_subdirectory(video_writer_simple_encode)