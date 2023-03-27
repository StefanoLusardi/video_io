set(TARGET_NAME video_reader_simple_decode)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

find_package(video_io REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE video_io::video_io)
