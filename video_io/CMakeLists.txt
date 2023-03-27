set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/video_io)

find_package(ffmpeg REQUIRED)

set(TARGET_SOURCES_PUBLIC
    include/video_io/api.hpp
    include/video_io/video_reader.hpp
    include/video_io/video_writer.hpp
)

set(TARGET_SOURCES_PRIVATE
    src/logger.hpp
    src/video_reader_hw.cpp
    src/video_reader_hw.hpp
    src/video_reader.cpp
    src/video_writer.cpp
)

set(TARGET_NAME video_io)
add_library(${TARGET_NAME} SHARED)
add_library(video_io::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_sources(${TARGET_NAME} PUBLIC ${TARGET_SOURCES_PUBLIC} PRIVATE ${TARGET_SOURCES_PRIVATE})
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${TARGET_NAME} PUBLIC VIDEO_IO)
target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} PRIVATE 
    ffmpeg::avformat 
    ffmpeg::avcodec 
    ffmpeg::swscale 
    ffmpeg::avutil
)

set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
set_target_properties(${TARGET_NAME} PROPERTIES PUBLIC_HEADER "${TARGET_SOURCES_PUBLIC}")

install(TARGETS ${TARGET_NAME} PUBLIC_HEADER DESTINATION include/video_io)
