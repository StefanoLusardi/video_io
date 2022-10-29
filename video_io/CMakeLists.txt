set(TARGET_NAME video_io)

set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/${TARGET_NAME})
find_package(ffmpeg REQUIRED)

set(TARGET_SOURCES_PUBLIC
    include/${PROJECT_NAME}/api.hpp
    include/${PROJECT_NAME}/raw_frame.hpp
    include/${PROJECT_NAME}/frame_queue.hpp
    include/${PROJECT_NAME}/video_reader.hpp
    include/${PROJECT_NAME}/video_writer.hpp
)

set(TARGET_SOURCES_PRIVATE
    src/logger.hpp
    src/video_reader_hw.cpp
    src/video_reader_hw.hpp
    src/video_reader.cpp
    src/video_writer.cpp
)

add_library(${TARGET_NAME} SHARED)
add_library(${TARGET_NAME}-static STATIC)

add_library(vio::${TARGET_NAME} ALIAS ${TARGET_NAME})
add_library(vio::${TARGET_NAME}-static ALIAS ${TARGET_NAME}-static)

target_sources(${TARGET_NAME} PUBLIC ${TARGET_SOURCES_PUBLIC} PRIVATE ${TARGET_SOURCES_PRIVATE})
target_sources(${TARGET_NAME}-static PUBLIC ${TARGET_SOURCES_PUBLIC} PRIVATE ${TARGET_SOURCES_PRIVATE})

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_compile_features(${TARGET_NAME}-static PUBLIC cxx_std_17)

target_compile_definitions(${TARGET_NAME} PUBLIC VIDEO_IO)
target_compile_definitions(${TARGET_NAME}-static PUBLIC VIDEO_IO)

target_include_directories(${TARGET_NAME} PUBLIC include)
target_include_directories(${TARGET_NAME}-static PUBLIC include)

target_link_libraries(${TARGET_NAME} PRIVATE ffmpeg::ffmpeg)
target_link_libraries(${TARGET_NAME}-static PRIVATE ffmpeg::ffmpeg)

set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
set_target_properties(${TARGET_NAME}-static PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
