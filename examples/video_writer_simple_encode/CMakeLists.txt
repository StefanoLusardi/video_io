set(TARGET_NAME video_writer_simple_encode)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE vio::video_io-static)
