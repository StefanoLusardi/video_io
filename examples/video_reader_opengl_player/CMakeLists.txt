set(TARGET_NAME video_reader_opengl_player)

find_package(glfw3)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE video_io
    PRIVATE glfw
)
