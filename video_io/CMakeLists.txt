set(TARGET_NAME video_io)

add_library(${TARGET_NAME} STATIC
    include/${PROJECT_NAME}/api.hpp
    include/${PROJECT_NAME}/raw_frame.hpp
    include/${PROJECT_NAME}/frame_queue.hpp
    include/${PROJECT_NAME}/video_reader.hpp
    include/${PROJECT_NAME}/video_writer.hpp
    
    src/logger.hpp
    src/video_reader_hw.cpp
    src/video_reader_hw.hpp
    src/video_reader.cpp
    src/video_writer.cpp
)

find_package(ffmpeg)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${TARGET_NAME} PUBLIC VIDEO_IO)
target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} PRIVATE ffmpeg::ffmpeg)
set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

add_executable(writer_example src/writer_example.cpp)
target_link_libraries(writer_example PRIVATE ffmpeg::ffmpeg)