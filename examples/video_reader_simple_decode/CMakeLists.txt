set(TARGET_NAME video_reader_simple_decode)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE vio::video_io-static)
