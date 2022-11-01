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

# Shared library
set(TARGET_NAME video_io)
add_library(${TARGET_NAME} SHARED)
add_library(vio::${TARGET_NAME} ALIAS ${TARGET_NAME})
target_sources(${TARGET_NAME} PUBLIC ${TARGET_SOURCES_PUBLIC} PRIVATE ${TARGET_SOURCES_PRIVATE})
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${TARGET_NAME} PUBLIC VIDEO_IO)
target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} PRIVATE ffmpeg::ffmpeg)
set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

# Static library
set(TARGET_NAME_STATIC video_io-static)
add_library(${TARGET_NAME_STATIC} STATIC)
add_library(vio::${TARGET_NAME_STATIC} ALIAS ${TARGET_NAME_STATIC})
target_sources(${TARGET_NAME_STATIC} PUBLIC ${TARGET_SOURCES_PUBLIC} PRIVATE ${TARGET_SOURCES_PRIVATE})
target_compile_features(${TARGET_NAME_STATIC} PUBLIC cxx_std_17)
target_compile_definitions(${TARGET_NAME_STATIC} PUBLIC VIDEO_IO)
target_include_directories(${TARGET_NAME_STATIC} PUBLIC include)
target_link_libraries(${TARGET_NAME_STATIC} PRIVATE ffmpeg::ffmpeg)
set_target_properties(${TARGET_NAME_STATIC} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
